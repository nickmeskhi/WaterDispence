#ifndef OP_FUNC_H
#define OP_FUNC_H

void prompt_func(int *state);
void select_clock(int *minute, int *hour, int *state, int *time);
void active_clock(int second, int minute, int hour, int *state, int time);

#endif // OP_FUNC_H
