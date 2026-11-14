#include <iostream>
#include <fstream>
#include <string>

#include "barcode/encoder.h"
#include "barcode/drawer.h"
#include "barcode/reader.h"
#include "barcode/decoder.h"
#include "barcode/camera.h"

using namespace std;

bool fileExists(const string& filename)
{
    ifstream file(filename);
    return file.good();
}

int main()
{
    int choice;

    while (true)
    {
        cout << "\n========== BARCODE SYSTEM ==========" << endl;
        cout << "1. Generate Barcode Image" << endl;
        cout << "2. Scan and Decode Barcode Image" << endl;
        cout << "3. Scan Barcode from Camera" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();           
            cin.ignore(1000, '\n');
            cout << "Invalid choice! Please select 1-4." << endl;
            continue;
        }
        cin.ignore(1000, '\n');

        
       // OPTION 1: GENERATE BARCODE
        if (choice == 1)
        {
            string number;

            cout << "\nEnter a 6-digit number: ";
            cin >> number;

            if (!isvalidInput(number))
            {
                cout << "Invalid number! Enter exactly 6 digits. First digit cannot be zero." << endl;
                continue;
            }

            string filename = makeBarcodeFilename(number);
            string bits = Encode(number);

            printConsole(bits);

            bool alreadyExists = false;

            if (createBarcodeImage(bits, filename, alreadyExists))
            {
                cout << "\nBarcode generated successfully!" << endl;
                cout << "Number: " << number << endl;
                cout << "Image: " << filename << endl;
            }
            else if (alreadyExists)
            {
                cout << "\nA barcode for " << number << " already exists: " << filename << endl;
            }
            else
            {
                cout << "\nError: could not save the image" << endl;
            }
        }
        // OPTION 2: SCAN AND DECODE BARCODE
        else if (choice == 2)
        {
            string filename;

            cout << "\nEnter barcode image filename (example: 123456 or barcodes/123456.png): ";
            cin >> filename;

            if (filename.size() < 6)
            {
                cout << "Invalid filename! It must be at least 6 characters (barcodes are 6-digit numbers)." << endl;
                continue;
            }

            // If the user typed no extension, add .png (all our barcodes are saved as .png)
            size_t slash = filename.find_last_of("/\\");
            string namePart = (slash == string::npos) ? filename : filename.substr(slash + 1);

            if (namePart.find('.') == string::npos)
            {
                filename += ".png";
            }

            // If only a name was typed (no folder), look inside barcodes/ folder
            if (filename.find('/') == string::npos && filename.find('\\') == string::npos)
            {
                filename = "barcodes/" + filename;
            }

            if (!fileExists(filename))
            {
                cout << "File not found: " << filename << endl;
                continue;
            }

            // Read image using OpenCV
            string row = readBarcodeRow(filename);

            if (row.empty())
            {
                cout << "Could not read barcode image!" << endl;
                continue;
            }

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

        //  OPTION 3: CAMERA SCAN TEST
        else if (choice == 3)
        {
            string number;

            if (captureFromCamera("camera.png", number))
            {
                cout << "Successfully decoded: " << number << endl;
            }
            else
            {
                cout << "Barcode could not be decoded." << endl;
            }
        }
        // OPTION 4: EXIT
        else if (choice == 4)
        {
            cout << "Exiting the program." << endl;
            break;
        }

        else
        {
            cout << "Invalid choice! Please select 1-4." << endl;
        }
    }

    return 0;
}