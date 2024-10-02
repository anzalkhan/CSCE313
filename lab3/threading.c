#include <threading.h>

void t_init()
{
        // TODO
        // Initialize all context entries to INVALID
        for (int i = 0; i < NUM_CTX; ++i) {
                contexts[i].state = INVALID;
        }

        // Set the current context index to 0 (or any other valid initial value)
        current_context_idx = 0;

        // Optionally, initialize the current context to be the main context
        // so that if we switch back to it, there's a valid context to switch to.
        if (getcontext(&contexts[current_context_idx].context) == -1) {
                perror("getcontext failed in t_init");
                exit(1);
        }

        // Mark the first context as VALID, which can be useful if needed in your workflow.
        contexts[current_context_idx].state = VALID;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
        // TODO
        // Find an available context slot
        for (volatile int i = 0; i < NUM_CTX; ++i) {
                if (contexts[i].state == INVALID) {
                        // Initialize the context
                        if (getcontext(&contexts[i].context) == -1) {
                                perror("getcontext failed");
                                return 1;
                        }

                        // Allocate memory for the stack
                        contexts[i].context.uc_stack.ss_sp = malloc(STK_SZ);
                        if (contexts[i].context.uc_stack.ss_sp == NULL) {
                                perror("malloc failed");
                                return 1;
                        }
                        contexts[i].context.uc_stack.ss_size = STK_SZ;
                        contexts[i].context.uc_stack.ss_flags = 0;

                        // Set the context to return to the current context after completion
                        contexts[i].context.uc_link = &contexts[current_context_idx].context;

                        // Create the context with the provided function and arguments
                        makecontext(&contexts[i].context, (ctx_ptr)foo, 2, arg1, arg2);

                        // Mark the context as VALID
                        contexts[i].state = VALID;
                        return 0; 
                }
        }

        // No available context slot
        return 1;
}

int32_t t_yield()
{
        // TODO
        int valid_contexts = 0;
        int next_context_idx = -1;

        // Find the next VALID context to switch to
        for (int i = 1; i < NUM_CTX; ++i) {
                int idx = (current_context_idx + i) % NUM_CTX;
                if (contexts[idx].state == VALID) {
                        valid_contexts++;
                        if (next_context_idx == -1) {
                                next_context_idx = idx;
                        }
                }
        }

        // If no other valid contexts found, return the number of valid contexts
        if (next_context_idx == -1) {
                return valid_contexts;
        }

        // Swap the current context with the next valid one
        int old_context_idx = current_context_idx;
        current_context_idx = (uint8_t) next_context_idx;

        // Perform the context switch
        if (swapcontext(&contexts[old_context_idx].context, &contexts[current_context_idx].context) == -1) {
                perror("swapcontext failed");
                exit(1);
        }

        return valid_contexts;
}

void t_finish()
{
        // TODO
        // Free the stack memory of the current context if it was allocated
        if (contexts[current_context_idx].context.uc_stack.ss_sp != NULL) {
                free(contexts[current_context_idx].context.uc_stack.ss_sp);
                contexts[current_context_idx].context.uc_stack.ss_sp = NULL;
        }

        // Mark the current context as DONE
        contexts[current_context_idx].state = DONE;

        // Try to find another valid context
        int next_context_idx = -1;

        for (int i = 1; i < NUM_CTX; ++i) {
                int idx = (current_context_idx + i) % NUM_CTX;
                if (contexts[idx].state == VALID) {
                        next_context_idx = idx;
                        break;
                }
        }

        // If no valid context exists, exit
        if (next_context_idx == -1) {
                exit(0); 
        }

        // Switch to the next valid context
        current_context_idx = (uint8_t)next_context_idx;

        // Set the new context, this function does not return
        if (setcontext(&contexts[current_context_idx].context) == -1) {
                perror("setcontext failed");
                exit(1);
        }
}
