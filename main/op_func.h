#ifndef OP_FUNC_H
#define OP_FUNC_H

void prompt_func(int *state);
void set_function(int *state, int *set_time, int *set_duration, int *set_day_count);
void active_function_clock(int *second, int *minute, int *hour, int *state, int *time, int *set_time, int *set_duration, int *set_day_count);
void select_clock(int *minute, int *hour, int *state, int *time);
void active_clock(int *second, int *minute, int *hour, int *state, int *time);

#endif // OP_FUNC_H