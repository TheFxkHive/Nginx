#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_str_t hls_opt;
}ngx_http_hls_loc_conf_t;


static ngx_int_t ngx_http_hls_init(ngx_conf_t *cf);

static void* ngx_http_hls_create_loc_conf(ngx_conf_t *cf);

static void* ngx_http_hls_opt_string(ngx_conf_t *cf, ngx_command_t *cmd, void* conf);

static void* ngx_http_hls_create_loc_merge(ngx_conf_t *cf, void* parent, void* chlid);
static ngx_command_t ngx_http_hls_commands[] = {

	{
		ngx_string("hls_opt"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,
		ngx_http_hls_opt_string,
		NGX_HTTP_LOC_OFFSET,
        offsetof(ngx_http_hls_loc_conf_t,hls_opt),
        NULL,
	},
    ngx_null_command
};


static int ngx_hls_request_times = 0;

static ngx_http_module_t ngx_http_hls_module_ctx = {
    NULL,
    ngx_http_hls_init,
    
    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_hls_create_loc_conf,
    ngx_http_hls_create_loc_merge,
};



static ngx_int_t ngx_http_hls_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if(j == NULL) {
        return NGX_ERROR;
    }
}

static void* ngx_http_hls_create_loc_conf(ngx_conf_t *cf)
{
	
}

static void* ngx_http_hls_create_loc_merge(ngx_conf_t * cf,void * parent,void * chlid)
{
	
}
static void* ngx_http_hls_opt_string(ngx_conf_t *cf, ngx_command_t *cmd, void* conf)
{

}
