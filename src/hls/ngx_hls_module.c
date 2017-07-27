#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define MEDIA_SEQUENCE 0

/*
typedef struct {
    int32_t pts;
    int64_t offset;
    int32_t pak_count;
    char    is_remain_frame;
    int32_t orig_pts;
} hint_item_t;
*/

#define HINT_ITEM_T_SIZE 21

#define TS_SLICE_TIME 2

typedef struct {
    int32_t       hint_version;
    int32_t       pmt_pid;
    int32_t       pcr_pid;
    int32_t       video_pid;
    int32_t       audio_pid;
    int16_t       video_stream_type;
    int16_t       audio_stream_type;
    int32_t       avg_bitrate;
    int32_t       hint_num;
    int32_t       status;
    uint32_t      update_time;
} hint_header_t;

typedef struct {
    ngx_flag_t hls;
    ngx_str_t  hls_version;
    ngx_flag_t hls_suffix;
    ngx_msec_t hls_duration;
    ngx_uint_t hls_m3u8_size;
} ngx_http_hls_loc_conf_t;

typedef struct {
    ngx_str_t m3u8;
    ngx_str_t name;
} ngx_http_hls_module_ctx_t;


static ngx_int_t ngx_http_hls_init(ngx_conf_t *cf);
static void * ngx_http_hls_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_hls_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t ngx_http_hls_module_handler(ngx_http_request_t *r);


static ngx_command_t  ngx_http_hls_commands[] = {
    { ngx_string("hls"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hls_loc_conf_t, hls),
      NULL },

    { ngx_string("hls_version"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hls_loc_conf_t, hls_version),
      NULL },

    { ngx_string("hls_suffix"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hls_loc_conf_t, hls_suffix),
      NULL },

    { ngx_string("hls_duration"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hls_loc_conf_t, hls_duration),
      NULL },

    { ngx_string("hls_m3u8_size"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hls_loc_conf_t, hls_m3u8_size),
      NULL },

    ngx_null_command
};

static ngx_http_module_t  ngx_http_hls_module_ctx = {
    NULL,                                        /* preconfiguration */
    ngx_http_hls_init,                           /* postconfiguration */

    NULL,                                       /* create main configuration */
    NULL,                                       /* init main configuration */

    NULL,                                       /* create server configuration */
    NULL,                                       /* merge server configuration */

    ngx_http_hls_create_loc_conf,               /* create location configuration */
    ngx_http_hls_merge_loc_conf                 /* merge location configuration */
};

ngx_module_t  ngx_http_hls_module = {
    NGX_MODULE_V1,
    &ngx_http_hls_module_ctx,              /* module context */
    ngx_http_hls_commands,                 /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_hls_module_set_header(ngx_http_request_t *r, ngx_list_t *headers,
                                u_char *key, ngx_str_t *value)
{
    size_t            key_size;
    ngx_uint_t        i, j;
    ngx_table_elt_t  *header;
    ngx_list_part_t  *part = &headers->part;

    if (!key) {
        return NGX_ERROR;
    }

    key_size = ngx_strlen(key);
    header = part->elts;

    for (i = 0; /* void */ ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (header[i].hash == 0) {
            continue;
        }

        if (key_size == header[i].key.len &&
            0 == ngx_strncasecmp(header[i].key.data, key, key_size))
        {
            if (!value->len) {
                for (j = i+1; j < part->nelts; j++) {
                    header[j-1] = header[j];
                }
                part->nelts--;
                return NGX_OK;
            }

            header[i].value.len = value->len;
            header[i].value.data = ngx_palloc(r->pool, value->len);
            if (!header[i].value.data) {
                return NGX_ERROR;
            }
            ngx_memcpy(header[i].value.data, value->data, value->len);
            return NGX_OK;
        }
    }

    if (!value->len) {
        return NGX_OK;
    }

    header = ngx_list_push(headers);
    if (header == NULL) {
        return NGX_ERROR;
    }

    header->lowcase_key = ngx_palloc(r->pool, key_size);
    if (header->lowcase_key == NULL) {
        return NGX_ERROR;
    }
    header->hash = ngx_hash_strlow(header->lowcase_key, key, key_size);

    header->key.len = key_size;
    header->key.data = ngx_palloc(r->pool, key_size);
    if (!header->key.data) {
        return NGX_ERROR;
    }
    ngx_memcpy(header->key.data, key, key_size);

    header->value.len = value->len;
    header->value.data = ngx_palloc(r->pool, value->len);
    if (!header->value.data) {
        return NGX_ERROR;
    }
    ngx_memcpy(header->value.data, value->data, value->len);

    return NGX_OK;
}

static ngx_int_t ngx_http_hls_update_uri(ngx_http_request_t *r)
{
    u_char       * uri   = r->uri.data;
    ngx_int_t    uri_len = r->uri.len;

    ngx_int_t    start_offset = 0, end_offset = 0;
    ngx_int_t    i, j, suffix = 0, prefix = 0;
    ngx_int_t    tes;
    ngx_int_t    rc;

    ngx_str_t    range;

    for(i = uri_len - 1; i >= 0; i--) {
        if(uri[i] == '.') {
            suffix = i;
            i = i - 1;
            tes = 1;
            while(uri[i] >= '0' &&  uri[i] <= '9') {
                end_offset = end + (uri[i] - '0') * tes;
                tes = tes * 10;
                i = i - 1;
            }
        }

        if(uri[i] == '_') {
            i = i -1;
            tes = 1;
            while(uri[i] >= '0' &&  uri[i] <= '9') {
                start_offset = start  + (uri[i] - '0') * tes;
                tes = tes * 10;
                i = i - 1;
            }
            if (uri[i] == '_' ) {
                prefix = i;
                break;
            } else {
                return NGX_ERROR;
            }
        }
    }

    if ( start_offset >= end_offset && end != 0) {
        return NGX_OK;
    }

    range.data = ngx_pcalloc(r->pool, sizeof("bytes=-") - 1 + 2 * NGX_OFF_T_LEN);
    if (!range.data) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                     "hls module ngx_pcalloc failed for range");
        return NGX_ERROR;
    }

    if (end_offset == 0) {
        range.len = ngx_sprintf(range.data, "bytes=%O-", start_offset) - range.data;
    } else {
        range.len = ngx_sprintf(range.data, "bytes=%O-%O", start_offset, end_offset) - range.data;
    }

    rc = ngx_http_hls_module_set_header(r, &r->headers_in.headers, (u_char *)"Range", &range);
    if (rc != NGX_OK) {
        return rc;
    }

    for(i = prefix, j = suffix; j < uri_len; i++, j++ ) {
        uri[i] = uri[j];
    }

    r->uri.data = uri;
    r->uri.len  = uri_len - (suffix - prefix);

    r->valid_unparsed_uri = 0;

    return NGX_OK; 

}


static void ngx_http_hls_module_post_handler(ngx_http_request_t *r)
{
    ngx_buf_t                    *buf;
    ngx_int_t                     rc;
    ngx_chain_t                   out;
    ngx_http_hls_module_ctx_t    *mctx;

    if (r->headers_out.status != NGX_HTTP_OK) {
        ngx_http_finalize_request(r, r->headers_out.status);
        return ;
    }

    mctx = ngx_http_get_module_ctx(r, ngx_http_hls_module);
    if(mctx == NULL) {
        return ;
    }

    buf = ngx_create_temp_buf(r->pool, mctx->m3u8.len);
    if (buf == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                "hls module create temp buf failed");
        return ;
    }

    ngx_snprintf(buf->pos, mctx->m3u8.len, "%V", &mctx->m3u8);
    buf->last = buf->pos + mctx->m3u8.len;
    buf->last_buf = 1;

    out.buf = buf;
    out.next = NULL;

    r->headers_out.content_length_n = mctx->m3u8.len;

    static ngx_str_t type = ngx_string("application/vnd.apple.mpegurl");

    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);

    rc = ngx_http_output_filter(r, &out);

    ngx_http_finalize_request(r, rc);
}

static ngx_int_t ngx_http_hls_subrequest_done(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
    ngx_http_hls_module_ctx_t    *mctx;
    ngx_http_request_t           *pr;
    ngx_http_hls_loc_conf_t      *mlcf = (ngx_http_hls_loc_conf_t *)data;
    hint_header_t                *hint_header;
    ngx_str_t                     args;

    ngx_int_t                     i;
    u_char                       *s;

    uint64_t                      start_offset, end_offset;
    uint32_t                      start_time, end_time;

    pr = r->parent;

    if(mlcf== NULL) {
        return NGX_ERROR;
    }

    mctx = ngx_http_get_module_ctx(pr, ngx_http_hls_module);
    if(mctx == NULL) {
        return NGX_ERROR;
    }

    args.data = NULL;
    args.len  = 0;

    if(mlcf->hls_suffix && r->args.len) {
        args.data = ngx_pcalloc(r->pool, r->args.len + 1);
        if (args.data == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                    "hls module ngx_pcalloc failed for args");
            return NGX_ERROR;
        }
        args.len = ngx_sprintf(args.data, "?%V", &r->args) - args.data;
    }

    pr->headers_out.status = r->headers_out.status;

    if (r->headers_out.status == NGX_HTTP_OK) {
        ngx_buf_t *recvBuf = &r->upstream->buffer;
        
        hint_header = (hint_header_t *) recvBuf->pos;

        recvBuf->pos = recvBuf->pos + sizeof(hint_header_t);

        mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, "#EXTM3U\n") - mctx->m3u8.data;
        mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, "#EXT-X-VERSION:%V\n", &mlcf->hls_version) - mctx->m3u8.data;
        mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, "#EXT-X-TARGETDURATION:%d\n", mlcf->hls_duration) - mctx->m3u8.data;
        mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, "#EXT-X-MEDIA-SEQUENCE:%d\n", MEDIA_SEQUENCE) - mctx->m3u8.data;

        start_offset = 0;
        start_time = 0;
        for(i = 1; i <= hint_header->hint_num; i++) {
            if (i * TS_SLICE_TIME % mlcf->hls_duration != 0) {
                continue;
            }
            s = recvBuf->pos + i * HINT_ITEM_T_SIZE;

            end_time = ((uint32_t)(*(s + 3)) << 24 | (uint32_t)(*(s + 2)) << 16 | \
                        (uint32_t)(*(s + 1)) << 8  | (uint32_t)(*(s)));

            end_offset = ((uint64_t)(*(s + 11)) << 56 | (uint64_t)(*(s + 10)) << 48 | \
                   (uint64_t)(*(s + 9))  << 40 | (uint64_t)(*(s + 8))  << 32 | \
                   (uint64_t)(*(s + 7))  << 24 | (uint64_t)(*(s + 6))  << 16 | \
                   (uint64_t)(*(s + 5))  << 8  | (uint64_t)(*(s + 4)));
            
            mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, \
                            "#EXTINF:%f,\n", (end_time - start_time) * 1.0 / 1000.0) - mctx->m3u8.data;
            mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, \
                            "%V_%d_%d.ts%V\n", &mctx->name, start_offset, end_offset - 1, &args) - mctx->m3u8.data;

            start_time = end_time;
            start_offset = end_offset;
        }

        if (hint_header->hint_num * TS_SLICE_TIME % mlcf->hls_duration != 0) {
            s = recvBuf->pos + (hint_header->hint_num - 1) * HINT_ITEM_T_SIZE;

            end_time = ((uint32_t)(*(s + 3)) << 24 | (uint32_t)(*(s + 2)) << 16 | \
                        (uint32_t)(*(s + 1)) << 8  | (uint32_t)(*(s)));
            end_offset = 0;

            mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, \
                            "#EXTINF:%f,\n", (end_time - start_time) * 1.0 / 1000.0) - mctx->m3u8.data;
            mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, \
                            "%V_%d_%d.ts%V\n", &mctx->name, start_offset, end_offset, &args) - mctx->m3u8.data;
        }

        mctx->m3u8.len = ngx_sprintf(mctx->m3u8.data + mctx->m3u8.len, "#EXT-X-ENDLIST\n") - mctx->m3u8.data;

    }

    pr->write_event_handler = ngx_http_hls_module_post_handler;

    return NGX_OK;
}

static ngx_int_t ngx_http_hls_send_subrequest(ngx_http_request_t *r, ngx_http_hls_loc_conf_t * mlcf)
{
    u_char                       *pos;
    ngx_str_t                     uri;
    ngx_int_t                     len;
    ngx_http_request_t           *sr;

    ngx_http_hls_module_ctx_t    *mctx;

    mctx = ngx_http_get_module_ctx(r, ngx_http_hls_module);
    if (mctx == NULL) {
        mctx = ngx_pcalloc(r->pool, sizeof(ngx_http_hls_module_ctx_t));
        if (mctx == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                        "hls moduel, pcalloc failed for ctx");
            return NGX_ERROR;
        }
    }

    /* init subrequest post callback */
    ngx_http_post_subrequest_t *psr = ngx_pcalloc(r->pool,sizeof(ngx_http_post_subrequest_t));
    if(psr == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                    "hls module subrequest malloc psr error, out of memory");
        return NGX_ERROR;
    }

    psr->handler = ngx_http_hls_subrequest_done;
    psr->data = mlcf;

    uri.data = ngx_pcalloc(r->pool, r->uri.len + 5);
    if (uri.data == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                    "hls module ngx_pcalloc failed for uri.data");
        return NGX_ERROR;
    }

    pos = ngx_strstrn(r->uri.data, ".m3u8", 5 - 1);
    if (pos == NULL) {
        return NGX_ERROR;
    } else {
        len = pos - r->uri.data;

        u_char* temp = pos;
        while(*temp != '/') {
            temp--;
        }
        mctx->name.data = temp + 1;
        mctx->name.len  = pos - temp - 1;
    }

    mctx->m3u8.data = ngx_pcalloc(r->pool, mlcf->hls_m3u8_size);
    if (mctx->m3u8.data == NULL ) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                    "hls module ngx_pcalloc failed for mctx->m3u8.data");
        return NGX_ERROR;
    }

    ngx_http_set_ctx(r, mctx, ngx_http_hls_module);

    uri.len = ngx_snprintf(uri.data, len, "%V", &r->uri) - uri.data;
    uri.len = ngx_sprintf(uri.data + len, ".ts.inx") - uri.data;

    if(ngx_http_subrequest(r, &uri, &r->args, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY) != NGX_OK){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, \
                    "hls module subrequest failed");
        return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t ngx_http_hls_module_handler(ngx_http_request_t *r)
{
    ngx_int_t                     rc;
    ngx_http_hls_loc_conf_t      *mlcf;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_hls_module);

    if (!mlcf->hls) {
        return NGX_DECLINED;
    }

    if (r->exten.len == 2 && ngx_strncasecmp(r->exten.data, (u_char *)"ts", 2) == 0) {
        rc = ngx_http_hls_update_uri(r);
        if (rc == NGX_OK) {
            return NGX_DECLINED;
        } else {
            return rc;
        }
    }

    if (r->exten.len != 4 || ngx_strncasecmp(r->exten.data, (u_char *)"m3u8", 4) != 0) {
        return NGX_DECLINED;
    }

    rc = ngx_http_hls_send_subrequest(r, mlcf);
    if(rc == NGX_OK) {
        return NGX_DONE;
    } else {
        return rc;
    }
}


static void *
ngx_http_hls_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_hls_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hls_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
    conf->hls_version.data = NULL;
    conf->hls_version.len = 0;
    */

    conf->hls = NGX_CONF_UNSET;
    conf->hls_suffix = NGX_CONF_UNSET;
    conf->hls_duration = NGX_CONF_UNSET_MSEC;
    conf->hls_m3u8_size = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_http_hls_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hls_loc_conf_t *prev = parent;
    ngx_http_hls_loc_conf_t *conf = child;

    ngx_conf_merge_value(conf->hls, prev->hls, 0);
    ngx_conf_merge_value(conf->hls_suffix, prev->hls_suffix, 0);
    ngx_conf_merge_msec_value(conf->hls_duration, prev->hls_duration, 10);
    ngx_conf_merge_uint_value(conf->hls_m3u8_size, prev->hls_m3u8_size, 4194304);
    ngx_conf_merge_str_value(conf->hls_version, prev->hls_version, "3");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_hls_init(ngx_conf_t *cf)
{
     ngx_http_handler_pt       *h;
     ngx_http_core_main_conf_t *cmcf;

     cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

     h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
     if (h == NULL) {
         return NGX_ERROR;
     }

     *h = ngx_http_hls_module_handler;
/*
     ngx_http_next_header_filter = ngx_http_top_header_filter;
     ngx_http_top_header_filter = ngx_http_hls_module_header_filter;

     ngx_http_next_body_filter = ngx_http_top_body_filter;
     ngx_http_top_body_filter = ngx_http_hls_module_body_filter;*/

    return NGX_OK;
}
