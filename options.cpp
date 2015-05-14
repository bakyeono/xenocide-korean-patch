#include "mem_check.h"

#include "options.h"

class OPTIONS options;

OPTIONS::OPTIONS()
{
  number[OPTION_LETTERS]=false;
  number[OPTION_EXPERIENCE]=false;
  number[OPTION_AUTO_AIM]=false;
  number[OPTION_DONT_LET_SWAP]=false;
  number[OPTION_CURSOR]=true;
  number[OPTION_GETALL]=true;
  number[OPTION_AIM_IF_NO_ENEMY]=true;
}

