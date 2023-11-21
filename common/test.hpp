#pragma once

#ifndef __RUN_TEST_H__
#define __RUN_TEST_H__
                                         
int runTest(int argc, const char * argv[]);

#ifdef _DEBUG
extern bool unittest_Generate_Exception_No_Reader;
extern bool unittest_Generate_Exception_Other;
extern bool unittest_Generate_Exception_Pin_Blocked;
extern bool unittest_Generate_Exception_Pin_3_attempts;
extern bool unittest_Generate_Exception_Pin_2_attempts;
extern bool unittest_Generate_Exception_Pin_1_attempt;
extern bool unittest_Generate_Exception_Src_Command_not_allowed;
extern bool unittest_Generate_Exception_Removed_Card;
extern bool unittest_Generate_Exception_Transaction_Fail;
extern bool unittest_Generate_Exception_PinPad_TimeOut;
extern bool unittest_Generate_Exception_PinPad_Cancel;
extern bool unittest_Generate_Exception_PIN_TOO_SHORT;
extern bool unittest_Generate_Exception_PIN_TOO_LONG;
extern bool unittest_Generate_Exception_PIN_Incorrect;
#endif

#endif //__RUN_TEST_H__

