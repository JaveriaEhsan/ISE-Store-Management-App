#include <iostream>
#include <fstream>
#include <string>
#include <cctype> 

#include "barcode/encoder.h"
#include "barcode/drawer.h"
#include "barcode/reader.h"
#include "barcode/decoder.h"

using namespace std;

bool fileExists(const string& filename)
{
    ifstream file(filename);
    return file.good();
}

bool isValidFilename(const string& filename)
{
     string name = filename;   
    for (int i = 0; i < name.size(); i++)
    {
        name[i] = tolower(name[i]);
    }
    // 4-letter extensions
    if (name.size() > 4)
    {
        string last4 = name.substr(name.size() - 4);
        if (last4 == ".png" || last4 == ".jpg" || last4 == ".bmp")
        {
            return true;
        }
    }
    // 5-letter extensions
    if (name.size() > 5)
    {
        string last5 = name.substr(name.size() - 5);
        if (last5 == ".jpeg")
        {
            return true;
        }
    }
    return false;
}

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
            cin.clear();           
            cin.ignore(1000, '\n');
            cout << "Invalid choice! Please select 1, 2, or 3." << endl;
            continue;
        }
        cin.ignore(1000, '\n');

        
       // OPTION 1: GENERATE BARCODE
        if (choice == 1)
        {
            string number;
            string filename;

            cout << "\nEnter a 6-digit number: ";
            cin >> number;

            if (!isvalidInput(number))
            {
                cout << "Invalid number! Enter exactly 6 digits. First digit cannot be zero." << endl;
                continue;
            }

            cout << "Enter barcode image filename (example: barcode1.png): ";
            cin >> filename;

            if (!isValidFilename(filename))
            {
                cout << "Invalid filename! Must end with .png, .jpg, .jpeg, or .bmp" << endl;
                continue;
            }

            if (fileExists(filename))
            {
                cout << "Error: a file with this name already exists. Choose another name." << endl;
                continue;
            }

            string bits = Encode(number);

            printConsole(bits);

            if (createBarcodeImage(bits, filename))
            {
                cout << "\nBarcode generated successfully!" << endl;
                cout << "Number: " << number << endl;
                cout << "Image: " << filename << endl;
            }
            else
            {
                cout << "\nError: could not save the image." << endl;
            }
        }

        // OPTION 2: SCAN AND DECODE BARCODE
        else if (choice == 2)
        {
            string filename;

            cout << "\nEnter barcode image filename (example: barcode1.png): ";
            cin >> filename;

            if(!isValidFilename(filename))
            {
                cout << "Invalid filename! Must end with .png, .jpg, .jpeg, or .bmp" << endl;
                continue;
            }

            if (!fileExists(filename))
            {
                cout << "File not found!" << endl;
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
