/*
 * File:   DM_Display.h
 * Author: Ed
 *
 * Created on July 15, 2021, 11:54 AM
 * Updated 2023-10-18 Aligned DM_AddChar2DisplayBuffer
 */

#ifndef DM_DISPLAY_H
#define DM_DISPLAY_H

/****************************************************************************
 Function
  DM_TakeInitDisplayStep

 Parameter
  None

 Returns
  bool: true when there are no more initialization steps to perform; false
        while there still more steps to be taken

 Description
  Initializes the MAX7219 4-module display performing 1 step for each call:
    First, bring put it in shutdown to disable all displays, return false
    Next fill the display RAM with Zeros to insure blanked, return false
    Then Disable Code B decoding for all digits, return false
    Then, enable scanning for all digits, return false
    The next setup step is to set the brightness to minimum, return false
    Copy our display buffer to the display, return false
    Finally, bring it out of shutdown and return true

Example
   while ( false == DM_TakeInitDisplayStep() )
   {} // note this example is for non-event-driven code
****************************************************************************/
bool DM_TakeInitDisplayStep(void);

/****************************************************************************
 Function
  DM_ClearDisplayBuffer

 Parameter
  None

 Returns
 Nothing (void)

 Description
  Clears the contents of the display buffer.

Example
   BDM_ClearDisplayBuffer();
****************************************************************************/
void DM_ClearDisplayBuffer(void);

/****************************************************************************
 Function
  DM_ScrollDisplayBuffer

 Parameter
  uint8_t: The number of Columns to scroll

 Returns
 Nothing (void)

 Description
  Scrolls the contents of the display buffer by the indicated number of
  columns.

Example
   DM_ScrollDisplayBuffer(4);
****************************************************************************/
void DM_ScrollDisplayBuffer(uint8_t NumCols2Scroll);

/****************************************************************************
 Function
  DM_TakeDisplayUpdateStep

 Parameter
  None

 Returns
  bool: true when all rows have been copied to the display; false otherwise

 Description
  Copies the contents of the display buffer to the MAX7219 controllers 1 row
  per call.

Example
   while (false == DM_TakeDisplayUpdateStep())
   {} // note this example is for non-event-driven code
****************************************************************************/
bool DM_TakeDisplayUpdateStep(void);

/****************************************************************************
 Function
  DM_AddChar2DisplayBuffer

 Parameter
  unsigned char: The character to be added to the display

 Returns
  Nothing (void)

 Description
  Copies the bitmap data from the font file into the rows of the frame buffer
  at the right-most character position in the buffer

Example
   DM_AddChar2DisplayBuffer('A');
****************************************************************************/
void DM_AddChar2DisplayBuffer(unsigned char Char2Display);

/****************************************************************************
 Function
  DM_PutDataIntoBufferRow

 Parameter
  uint32_t: The new row data to be stored in the display buffer
  uint8_t:  The row (0->7) into which the data will be stored.

 Returns
  bool: true for a legal row number; false otherwise

 Description
  Copies the raw data from the Data2Insert parameter into the specified row
  of the frame buffer

Example
   DM_PutDataInBufferRow(0x00000001, 0);
****************************************************************************/
bool DM_PutDataIntoBufferRow(uint32_t Data2Insert, uint8_t WhichRow);
bool DM_PutDataIntoBufferRow_v2(uint32_t Data2Insert, uint8_t WhichRow);

/****************************************************************************
 Function
  DM_QueryRowData

 Parameter
  uint8_t: The row of the display buffer to be queried
  uint32_t *: pointer to variable to hold the data from the buffer

 Returns
  bool: true for a legal row number; false otherwise

 Description
  copies the contents of the specified row of the frame buffer into the
 location pointed to by pReturnValue

Example
   DM_QueryRowData(0,&ReturnedValue);
****************************************************************************/
bool DM_QueryRowData(uint8_t RowToQuery, uint32_t *pReturnValue);

void SPI_SendToAllModules(uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4);

bool DM_test(void);

void DM_AddLive2DisplayBuffer(uint8_t liveLevel);

void DM_CenterDisplayText(const char *text, uint8_t textLength);

#endif /* DM_DISPLAY_H */
