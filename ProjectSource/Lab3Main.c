#include <xc.h>
#include "PIC32_SPI_HAL.h"
#include "DM_Display.h"
#define DIGIT_0_CMD 0x01  // Command for row 0 (Digit 0) on MAX7219

#include <stdio.h>
#include <stdint.h>
#include "DM_Display.h"
#include "PIC32_SPI_HAL.h"

// Function prototypes for the test harness
void Test_InitializeDisplay(void);
void Test_PutDataIntoBuffer(void);
void Test_QueryBuffer(void);
void Test_DisplayUpdate(void);

int run_tests(void)
{
    // Step 1: Initialize the Display and SPI HAL
    Test_InitializeDisplay();

    // Step 2: Put Data into the Display Buffer
    Test_PutDataIntoBuffer();

    // Step 3: Query the Display Buffer to verify correctness
    Test_QueryBuffer();

    // Step 4: Update the Display (this will send the buffer to the actual hardware)
    Test_DisplayUpdate();



    return 0;
}

// Function to initialize the display using the HAL
void Test_InitializeDisplay(void)
{
    
    // Now initialize the display in a loop until it is fully initialized
    while (false == DM_TakeInitDisplayStep())
    {
        // Continue calling the function to initialize the display step by step
    }
    
    printf("Display Initialized.\n");
}

// Function to put test data into the display buffer
void Test_PutDataIntoBuffer(void)
{
    // Insert a test pattern in row 0 (e.g., 0xA5A5A5A5)
    DM_PutDataIntoBufferRow(0xA5A5A5A5, 0);
    printf("Inserted data into row 0: 0xA5A5A5A5\n");

    // Insert another pattern in row 1 (e.g., 0x5A5A5A5A)
    DM_PutDataIntoBufferRow(0x5A5A5A5A, 1);
    printf("Inserted data into row 1: 0x5A5A5A5A\n");

    // Add more test patterns as needed
}

// Function to query the display buffer and verify data
void Test_QueryBuffer(void)
{
    uint32_t rowData;

    // Query row 0
    if (DM_QueryRowData(0, &rowData))
    {
        printf("Queried row 0: 0x%08X\n", rowData);
    }
    else
    {
        printf("Error querying row 0.\n");
    }

    // Query row 1
    if (DM_QueryRowData(1, &rowData))
    {
        printf("Queried row 1: 0x%08X\n", rowData);
    }
    else
    {
        printf("Error querying row 1.\n");
    }

    // Add more queries as needed
}

// Function to update the display
void Test_DisplayUpdate(void)
{
    // Update the display buffer to the hardware row by row
    while (false == DM_TakeDisplayUpdateStep())
    {
        // Continue updating the display
    }
    printf("Display updated with current buffer contents.\n");
}

void Init_MAX7219(void) {
    // Step 1: Put all displays in shutdown (disable all)
    SPI_SendToAllModules(0x0C << 8 | 0x00, 0x0C << 8 | 0x00, 0x0C << 8 | 0x00, 0x0C << 8 | 0x00);

    // Step 2: Disable Code B decoding for all digits in all modules
    SPI_SendToAllModules(0x09 << 8 | 0x00, 0x09 << 8 | 0x00, 0x09 << 8 | 0x00, 0x09 << 8 | 0x00);

    // Step 3: Enable scanning for all digits in all modules (0�??7)
    SPI_SendToAllModules(0x0B << 8 | 0x07, 0x0B << 8 | 0x07, 0x0B << 8 | 0x07, 0x0B << 8 | 0x07);

    // Step 4: Set brightness to minimum for all modules
    SPI_SendToAllModules(0x0A << 8 | 0x00, 0x0A << 8 | 0x00, 0x0A << 8 | 0x00, 0x0A << 8 | 0x00);

    // Step 5: Clear the display (write all rows with data 0x00)
    for (int digit = 1; digit <= 8; digit++) {
        SPI_SendToAllModules(digit << 8 | 0x00, digit << 8 | 0x00, digit << 8 | 0x00, digit << 8 | 0x00);
    }

    // Step 6: Bring all displays out of shutdown (enable all)
    SPI_SendToAllModules(0x0C << 8 | 0x01, 0x0C << 8 | 0x01, 0x0C << 8 | 0x01, 0x0C << 8 | 0x01);
}


int run_test_2_2(void) {
    // Initialize the display using the Dot Matrix HAL
    while (false == DM_TakeInitDisplayStep()) {
        // Continue calling to fully initialize the display
    }

    // Variables to track the row and LED position
    uint8_t currentRow = 0;  // Start from the top row
    uint32_t ledPosition = 0x80000000; // Single LED at the rightmost position (binary 1000...)

    // Infinite loop to scroll the LED continuously
    while (1) {
        // Insert the LED into the current row of the display buffer
        DM_PutDataIntoBufferRow(ledPosition, currentRow);

        // Update the display with the new LED position
        while (false == DM_TakeDisplayUpdateStep()) {
            // Keep updating the display until all rows are updated
        }

        // Shift the LED one position to the left
        ledPosition >>= 1;

        // Check if the LED has scrolled off the left edge
        if (ledPosition == 0) {
            // Reset the LED to the rightmost position
            ledPosition = 0x80000000;

            // Move to the next row
            currentRow++;

            // If the LED has scrolled off the bottom row, reset to the top row
            if (currentRow > 7) {
                currentRow = 0;
            }

            // Clear the current row after moving the LED to the new row
            DM_ClearDisplayBuffer();
        }

        // Delay to control scrolling speed
        for (volatile uint32_t i = 0; i < 100000; i++); // Adjust delay as needed
    }

    return 0;
}

int main(void) {
    SPISetup_BasicConfig(SPI_SPI1);
    SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
    SPI1BUF;
    SPISetEnhancedBuffer(SPI_SPI1, 1);
    SPISetup_SetBitTime(SPI_SPI1, 10000);
    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
    SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE);
    SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI);
    SPI1CONbits.FRMPOL = 0;
    SPISetup_EnableSPI(SPI_SPI1);

    //Init_MAX7219();
    //SPI_SendToAllModules(DIGIT_0_CMD << 8 | 0x80, DIGIT_0_CMD << 8 | 0x00,DIGIT_0_CMD << 8 | 0x00, DIGIT_0_CMD << 8 | 0x00);

    //run_tests()
    run_test_2_2()
    //run_tests();
    while (1)
    {}
    return 0;
}
