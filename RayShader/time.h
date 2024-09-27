/*******************************\
| A small unitlity to get both  |
| windows and linux times.      |
\*******************************/

#pragma once

void time_init(void);
float time_dt(void); // time_init() must be called before this
