#ifndef SWOOLE_CORO_INCLUDE_C_H_
#define SWOOLE_CORO_INCLUDE_C_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_MAX_CORO_NUM 3000
#define DEFAULT_STACK_SIZE   8192
#define MAX_CORO_NUM_LIMIT   0x80000

#define CORO_END 0
#define CORO_YIELD 1
#define CORO_LIMIT 2
#define CORO_SAVE 3

typedef enum
{
    SW_CORO_YIELD = 0, SW_CORO_SUSPENDED, SW_CORO_RUNNING, SW_CORO_END,
} sw_coro_state;

#define SW_EX_CV_NUM(ex, n) (((zval ***)(((char *)(ex)) + ZEND_MM_ALIGNED_SIZE(sizeof(zend_execute_data)))) + n)
#define SW_EX_CV(var) (*SW_EX_CV_NUM(execute_data, var))

typedef struct _php_context php_context;
typedef struct _coro_task coro_task;
typedef struct stCoRoutine_t stCoRoutine_t;

typedef enum
{
    SW_CORO_CONTEXT_RUNNING, SW_CORO_CONTEXT_IN_DELAYED_TIMEOUT_LIST, SW_CORO_CONTEXT_TERM
} php_context_state;

struct _php_context
{
    zval **current_coro_return_value_ptr_ptr;
    zval *current_coro_return_value_ptr;
    zval coro_params;
    void (*onTimeout)(struct _php_context *cxt);
    void *private_data;
    zval **current_eg_return_value_ptr_ptr;
    zend_execute_data *current_execute_data;
    zval *current_vm_stack_top;
    zval *current_vm_stack_end;
    zval *allocated_return_value_ptr;
    coro_task *current_task;
    zend_vm_stack current_vm_stack;
    php_context_state state;
};

typedef struct _coro_global
{
    uint32_t coro_num;
    uint32_t max_coro_num;
    uint32_t stack_size;
    zend_vm_stack origin_vm_stack;
    zval *origin_vm_stack_top;
    zval *origin_vm_stack_end;
    zval *allocated_return_value_ptr;
    zend_execute_data *origin_ex;
    coro_task *root_coro;
    coro_task *current_coro;
    coro_task *next_coro;
    volatile zend_bool pending_interrupt;
    zend_bool require;
    zend_bool active;
    int call_stack_size;
} coro_global;

typedef struct _php_args
{
    zend_fcall_info_cache *fci_cache;
    zval **argv;
    int argc;
    zval *retval;
    void *post_callback;
    void *params;
} php_args;

struct _coro_task
{
    int cid;
    stCoRoutine_t *co;
    sw_coro_state state;
    zend_execute_data *execute_data;
    zend_vm_stack stack;
    zval *vm_stack_top;
    zval *vm_stack_end;
    /**
     * user coroutine
     */
    zval *function;
    time_t start_time;
    void (*post_callback)(void *param);
    void *post_callback_params;
    php_args args;
};

typedef struct _swTimer_coro_callback
{
    int ms;
    int cli_fd;
    long *timeout_id;
    void* data;
} swTimer_coro_callback;

extern coro_global COROG;
#define get_current_cid()      (COROG.current_coro == NULL ? -1 : COROG.current_coro->cid)

int coro_init(TSRMLS_D);
void coro_destroy(TSRMLS_D);
void coro_check(TSRMLS_D);
void coro_handle_timeout();

#define coro_create(op_array, argv, argc, retval, post_callback, param) \
        libco_create(op_array, argv, argc, *retval, post_callback, param)
#define coro_save(sw_php_context) \
        sw_coro_save(return_value, sw_php_context);
#define coro_resume(sw_current_context, retval, coro_retval) \
        sw_coro_resume(sw_current_context, retval, *coro_retval)
#define coro_resume_parent(sw_current_context, retval, coro_retval) \
        sw_coro_resume_parent(sw_current_context, retval, coro_retval)
#define coro_yield() sw_coro_yield()

int libco_create(zend_fcall_info_cache *fci_cache, zval **argv, int argc, zval *retval, void *post_callback, void *params);

int sw_coro_create(zend_fcall_info_cache *op_array, zval **argv, int argc, zval *retval, void *post_callback, void *param);
int sw_coro_yield();
void sw_coro_close();
int sw_coro_resume(php_context *sw_current_context, zval *retval, zval *coro_retval);
int sw_coro_resume_parent(php_context *sw_current_context, zval *retval, zval *coro_retval);
int sw_coro_save(zval *return_value, php_context *sw_php_context);
int sw_coro_test();

int php_swoole_add_timer_coro(int ms, int cli_fd, long *timeout_id, void* param, swLinkedList_node **node TSRMLS_DC);
int php_swoole_clear_timer_coro(long id TSRMLS_DC);

#ifdef __cplusplus
}  /* end extern "C" */
#endif
#endif  /* SWOOLE_CORO_INCLUDE_C_H_ */
