
#include <iostream>
#include <string>

#include "barcode/encoder.h"
#include "barcode/drawer.h"
#include "barcode/reader.h"
#include "barcode/decoder.h"

using namespace std;

int main()
{
    int choice;

    while (true)
    {
        cout << "\n========== BARCODE SYSTEM ==========" << endl;
        cout << "1. Generate Barcode Image" << endl;
        cout << "2. Scan and Decode Barcode Image" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid choice!" << endl;
            return 0;
        }

        // OPTION 1: GENERATE BARCODE
        if (choice == 1)
        {
            string number;
            string filename;

            cout << "\nEnter a 6-digit number: ";
            cin >> number;

            if (!isvalidInput(number))
            {
                cout << "Invalid number! Enter exactly 6 digits.";
                cout << " First digit cannot be zero." << endl;
                continue;
            }

            cout << "Enter image filename (example: barcode1.png): ";
            cin >> filename;

            string bits = Encode(number);

            cout << "\nGenerated barcode bits:" << endl;
            printConsole(bits);

            if (createBarcodeImage(bits, filename))
            {
                cout << "\nBarcode generated successfully!" << endl;
                cout << "Number: " << number << endl;
                cout << "Image: " << filename << endl;
            }
        }

        // OPTION 2: SCAN AND DECODE BARCODE
        else if (choice == 2)
        {
            string filename;

            cout << "\nEnter barcode image filename: ";
            cin >> filename;

            // Read image using OpenCV
            string row = readBarcodeRow(filename);

            if (row.empty())
            {
                cout << "Could not read barcode image!" << endl;
                continue;
            }

            cout << "Read row size: " << row.size() << endl;

            // Decode barcode
            string decodedNumber;
            string error;

            if (decodeRow(row, decodedNumber, error))
            {
                cout << "\n========== SCAN RESULT ==========" << endl;
                cout << "Decoded number: " << decodedNumber << endl;
                cout << "Barcode scanned successfully!" << endl;
            }
            else
            {
                cout << "\nDecoding failed: " << error << endl;
            }
        }

        // OPTION 3: EXIT
        else if (choice == 3)
        {
            cout << "Exiting Barcode System..." << endl;
            break;
        }

        else
        {
            cout << "Invalid choice! Please select 1, 2, or 3." << endl;
        }
    }

    return 0;
}
