#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_flag_t hls_opt;
}ngx_http_hls_loc_conf_t;


static ngx_int_t ngx_http_hls_init(ngx_conf_t *cf);

static void* ngx_http_hls_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_hls_merge_loc_conf(ngx_conf_t *cf, void* parent, void* child);
static ngx_int_t ngx_http_hls_handler(ngx_http_request_t *r);


static ngx_command_t ngx_http_hls_commands[] = {

	{
		ngx_string("hls_opt"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG|NGX_CONF_TAKE1,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hls_loc_conf_t,hls_opt),
        NULL,
	},
    ngx_null_command
};

static ngx_http_module_t ngx_http_hls_module_ctx = {
    NULL,
    ngx_http_hls_init,
    
    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_hls_create_loc_conf,
    ngx_http_hls_merge_loc_conf
};

ngx_module_t ngx_http_hls_module = {
	NGX_MODULE_V1,
	&ngx_http_hls_module_ctx,
	ngx_http_hls_commands,
	NGX_HTTP_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
	
};
void ngx_http_hls_get_path(ngx_http_request_t *r,ngx_str_t *path)
{
	ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
	
	if(!clcf) {
		return;
	}
	
	ngx_cpystrn((u_char*)path->data, (u_char*)clcf->root.data, clcf->root.len + 1);
	path->len = clcf->root.len;
	
}
void* ngx_http_hls_get_m3u8(ngx_http_request_t *r)
{
	ngx_str_t * m3u8;
	ngx_str_t path;
	ngx_dir_t dir;
	ngx_str_t filename;
	u_char* dru_pos;
	m3u8 = ngx_palloc(r->pool,sizeof(ngx_str_t));
	if(!m3u8) {
		return NULL;
	}
	m3u8->data = ngx_palloc(r->pool, 4096);
	if(!m3u8->data) {
		return NULL;
	}
	
	ngx_memset(m3u8->data, 0, 4096);
	path.data = ngx_palloc(r->pool, 1024);
	path.len = 0;
	if(!path.data) {
		return NULL;
	}
	ngx_http_hls_get_path(r, &path);
	if (ngx_open_dir(&path, &dir) == NGX_ERROR) {
		return NULL;
	}

	m3u8->len = ngx_sprintf(m3u8->data + m3u8->len, "#EXTM3U\n") - m3u8->data;
	m3u8->len = ngx_sprintf(m3u8->data + m3u8->len, "#EXT-X-VERSION:2\n") - m3u8->data;      
	m3u8->len = ngx_sprintf(m3u8->data + m3u8->len, "#EXT-X-TARGETDURATION:10\n") - m3u8->data;       
	m3u8->len = ngx_sprintf(m3u8->data + m3u8->len, "#EXT-X-MEDIA-SEQUENCE:1\n") -  m3u8->data;
	for( ; ; ) {
		if (ngx_read_dir(&dir) == NGX_ERROR) {
            break;
        }
		filename.data = ngx_de_name(&dir);
		filename.len = ngx_de_namelen(&dir);
		dru_pos = (u_char* )ngx_strchr(filename.data, '_');
		if (filename.data[0] == '.' || filename.len < 7) {
			continue;
		}
		/*
		solve "duration|ts_name.ts" this file format
		*/

		if (filename.data[filename.len - 1] == 's' &&
			filename.data[filename.len - 2] == 't' &&
			filename.data[filename.len - 3] == '.' &&
			dru_pos != (u_char*)filename.data){
			
			
			m3u8->len = ngx_sprintf(m3u8->data+m3u8->len, "#EXTINF:%d,\n", 
				(int)ngx_atoi(filename.data,dru_pos-filename.data)) - m3u8->data;
			m3u8->len = ngx_sprintf(m3u8->data+m3u8->len, "%s\n", 
				filename.data) - m3u8->data;					
		}

		
	}
	m3u8->len = ngx_sprintf(m3u8->data+m3u8->len, "#EXT-X-ENDLIST\n") - m3u8->data;
	return m3u8;
	
}
static ngx_int_t ngx_http_hls_handler(ngx_http_request_t *r)
{
	ngx_http_hls_loc_conf_t *locf;
	ngx_str_t				*m3u8;
	ngx_buf_t 				*buf;
	ngx_int_t 				rc;
	ngx_chain_t 			out;
	locf = ngx_http_get_module_loc_conf(r,ngx_http_hls_module);

	if(!locf->hls_opt) {
		return NGX_DECLINED;
	}

	m3u8 = ngx_http_hls_get_m3u8(r);

	if(!m3u8) {
		return NGX_DECLINED;
	}
	if(r->exten.len == 2 && ngx_strncasecmp(r->exten.data, (u_char *)"ts", 2) == 0) {
 		return NGX_DECLINED;
 	}
 	if(r->exten.len != 4 || ngx_strncasecmp(r->exten.data, (u_char *)"m3u8", 4) != 0) {
 		return NGX_DECLINED;
 	}
	buf = ngx_create_temp_buf(r->pool, m3u8->len);
	ngx_snprintf(buf->pos, m3u8->len, "%V", m3u8);
	buf->last = buf->pos + m3u8->len;
	buf->last_buf = 1; 
	out.buf = buf;
	out.next = NULL;
	static ngx_str_t type = ngx_string("application/vnd.apple.mpegurl");
	r->headers_out.content_length_n = m3u8->len;

	r->headers_out.content_type = type;
	r->headers_out.status = NGX_HTTP_OK;
	rc = ngx_http_send_header(r);
	rc = ngx_http_output_filter(r, &out);   
	ngx_http_finalize_request(r, rc);
	
	
	return NGX_OK;
}

static ngx_int_t ngx_http_hls_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if(h == NULL) {
        return NGX_ERROR;
    }
	
	*h = ngx_http_hls_handler;
	return NGX_OK;
}

static void* ngx_http_hls_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_hls_loc_conf_t *conf;
	conf = ngx_pcalloc(cf->pool,sizeof(ngx_http_hls_loc_conf_t));

	if(conf == NULL) {
		return NULL;
	}

	conf->hls_opt = NGX_CONF_UNSET;

	return conf;
}
static char* ngx_http_hls_merge_loc_conf(ngx_conf_t *cf, void* parent, void* child)
{
	ngx_http_hls_loc_conf_t *prev = parent;
	ngx_http_hls_loc_conf_t *conf = child;

	ngx_conf_merge_value(conf->hls_opt, prev->hls_opt, 0);

	return NGX_CONF_OK;
}


