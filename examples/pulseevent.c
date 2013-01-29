#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <pthread.h>

static int m_state = 0;
static pa_context *m_pa_ctx = NULL;

static void m_pa_state_cb(pa_context *c, void *userdata);
static void *m_pa_mainloop_cb(void *arg);
static void *m_pa_context_subscribe_cb(pa_context *c, 
                                       pa_subscription_event_type_t t, 
                                       uint32_t idx, 
                                       void *userdata);

int main(int argc, char *argv[]) {
    pthread_t thread;

    pthread_create(&thread, NULL, m_pa_mainloop_cb, NULL);

    sleep(3);
    if (m_pa_ctx) {
        pa_context_set_sink_mute_by_index(m_pa_ctx, 1, 1, NULL, NULL);
    }

    while (1) {
        sleep(1);
    }

    return 0;
}

/* http://freedesktop.org/software/pulseaudio/doxygen/subscribe.html */
static void *m_pa_mainloop_cb(void *arg) {
    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml = NULL;
    pa_mainloop_api *pa_mlapi = NULL;
    pa_operation *pa_op = NULL;
    int pa_ready;

RE_CONN:
    // We'll need these state variables to keep track of our requests
    m_state = 0;
    pa_ready = 0;

    // Create a mainloop API and connection to the default server
    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    m_pa_ctx = pa_context_new(pa_mlapi, "test");

    // This function connects to the pulse server
    pa_context_connect(m_pa_ctx, NULL, 0, NULL);

    // This function defines a callback so the server will tell us it's state.
    // Our callback will wait for the state to be ready.  The callback will
    // modify the variable to 1 so we know when we have a connection and it's
    // ready.
    // If there's an error, the callback will set pa_ready to 2
    pa_context_set_state_callback(m_pa_ctx, m_pa_state_cb, &pa_ready);

    // Now we'll enter into an infinite loop until we get the data we receive
    // or if there's an error
    for (;;) {
        // We can't do anything until PA is ready, so just iterate the mainloop
        // and continue
        if (pa_ready == 0) {
            pa_mainloop_iterate(pa_ml, 1, NULL);
            continue;
        }
        // We couldn't get a connection to the server, so exit out
        if (pa_ready == 2) {
            printf("fail to connect to pulse server\n");
            printf("try to reconnect to pulse server\n");
            pa_context_disconnect(m_pa_ctx);                                      
            pa_context_unref(m_pa_ctx);                                           
            pa_mainloop_free(pa_ml);
            /* wait for a while to reconnect to pulse server */
            sleep(3);
            goto RE_CONN;
        }
        // At this point, we're connected to the server and ready to make
        // requests
        switch (m_state) {
            // State 0: we haven't done anything yet
            case 0:
                printf("try to enable event notification\n");
                pa_op = pa_context_subscribe(m_pa_ctx,
                        PA_SUBSCRIPTION_MASK_ALL,
                        NULL, 
                        NULL);
                pa_context_set_subscribe_callback(m_pa_ctx,                       
                    m_pa_context_subscribe_cb,                                    
                    NULL);
                m_state++;
                break;
            case 1:
                usleep(100);
                break;
            case 2:
                // Now we're done, clean up and disconnect and return
                printf("disconnect pulse server\n");
                pa_context_disconnect(m_pa_ctx);
                pa_context_unref(m_pa_ctx);
                pa_mainloop_free(pa_ml);
                return NULL;
            default:
                // We should never see this state
                fprintf(stderr, "in state %d\n", m_state);
                return NULL;
        }
        // Iterate the main loop and go again.  The second argument is whether
        // or not the iteration should block until something is ready to be
        // done.  Set it to zero for non-blocking.
        pa_mainloop_iterate(pa_ml, 1, NULL);
    }

    return NULL;
}

// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
static void m_pa_state_cb(pa_context *c, void *userdata) {
    pa_context_state_t state;
    int *pa_ready = userdata;
    
    state = pa_context_get_state(c);
    switch (state) {
        // There are just here for reference
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            *pa_ready = 2;
            break;
        case PA_CONTEXT_READY:
            *pa_ready = 1;
            break;
    }
}

static void *m_pa_context_subscribe_cb(pa_context *c, 
                                       pa_subscription_event_type_t t, 
                                       uint32_t idx, 
                                       void *userdata) 
{
    printf("event notified %d %d\n", t, idx);
}
