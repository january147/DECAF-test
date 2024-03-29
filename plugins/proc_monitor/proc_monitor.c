#include "DECAF_types.h"
#include "DECAF_main.h"
#include "DECAF_callback.h"
#include "DECAF_callback_common.h"
#include "vmi_callback.h"
#include "utils/Output.h"
#include "DECAF_target.h"

const char* plugin_name = "proc_monitor";

//basic stub for plugins
static plugin_interface_t my_interface;
static DECAF_Handle processbegin_handle = DECAF_NULL_HANDLE;
static DECAF_Handle processexit_handle = DECAF_NULL_HANDLE;

char targetname[512];

/* This callback is invoked when a new process starts in the guest OS. */
static void my_loadmainmodule_callback(VMI_Callback_Params* params) { 
    if(strcmp(params->cp.name,targetname) == 0)
        DECAF_printf("[%s]process %s starts\n",plugin_name, params->cp.name); 
} /* Handler to implement the command monitor_proc. */

static void my_proc_exit_callback(VMI_Callback_Params* params) {
    if (0 == strcmp(params->cp.name, targetname)) {
        DECAF_printf("[%s]process %s destoried\n", plugin_name, params->cp.name);
    }
}


/*command handler*/
void do_monitor_proc(Monitor *mon, const QDict* qdict) { 
    /* Copy the name of the process to be monitored to targetname. */ 
    if ((qdict != NULL) && (qdict_haskey(qdict, "procname"))) {
        strncpy(targetname, qdict_get_str(qdict, "procname"), 512); 
    } 
    targetname[511] = '\0'; 
}



// init and cleanup
static int my_init(void) { 
    DECAF_printf("Hello World\n");
    //register for process create and process remove events  
    processbegin_handle = VMI_register_callback(VMI_CREATEPROC_CB, &my_loadmainmodule_callback, NULL);
    if (processbegin_handle == DECAF_NULL_HANDLE) {
        DECAF_printf("Could not register for the create or remove proc events\n");  
    }

    processexit_handle = VMI_register_callback(VMI_REMOVEPROC_CB, &my_proc_exit_callback, NULL);
    if (processexit_handle == DECAF_NULL_HANDLE) {
        DECAF_printf("Could not register for the create or remove proc events\n");  
    }
    
    return 0;
}
/* This function is invoked when the plugin is unloaded. */
 static void my_cleanup(void) {
    DECAF_printf("Bye world\n");  /* Unregister for the process start and exit callbacks. */
    if (processbegin_handle != DECAF_NULL_HANDLE) {
        VMI_unregister_callback(VMI_CREATEPROC_CB, processbegin_handle);  
        processbegin_handle = DECAF_NULL_HANDLE; 
    }
    if (processexit_handle != DECAF_NULL_HANDLE) {
        VMI_unregister_callback(VMI_REMOVEPROC_CB, processexit_handle);  
        processexit_handle = DECAF_NULL_HANDLE; 
    }
 } 
 
 /* Commands supported by the plugin. Included in plugin_cmds.h */
 static mon_cmd_t my_term_cmds[] = {
    { 
        .name = "monitor_proc",
        .args_type = "procname:s",
        .mhandler.cmd = do_monitor_proc,
        .params = "[procname]",
        .help = "Run the tests with program [procname]" },
        {NULL, NULL, },
 };

/* This function registers the plugin_interface with DECAF.
The interface is used to register custom commands, let DECAF know which cleanup function to call upon plugin unload, etc,.*/
plugin_interface_t* init_plugin(void) {
    my_interface.mon_cmds = my_term_cmds;
    my_interface.plugin_cleanup = &my_cleanup;
    my_init();
    return (&my_interface);
}