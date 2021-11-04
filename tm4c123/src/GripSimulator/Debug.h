#ifndef _DEBUG_H_
#define _DEBUG_H_

// debugging

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))


void LogicAnalyzerTask(void);

void ScopeTask(void);


#endif
