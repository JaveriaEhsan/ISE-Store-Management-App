# Superstore App - 3rd Semester ISE Project at FAST

A standalone C++ desktop application for a small store to manage inventory
and record sales with minimal typing. Cashiers scan barcodes with a camera
to build orders and generate bills. Managers view products and barcodes and
manage the inventory.

## Group Members
- Ammara Ishtiaq - [Increment 2 role, e.g. Product model, ProductRepository, product and inventory screens]
- Meerab Babar - [Increment 2 role, e.g. Order/Bill models and repositories, manager window]
- Ayesha Javed - [Increment 2 role, e.g. BarcodeScanner, CameraWidget, ScanDialog]
- Rimsha Mushtaq - [Increment 2 role, e.g. BillCalculator, OrderService, payment and bill dialogs]
- Javeria Ehsan - Qt setup, CMake, main window, cashier window, integration, GitHub, testing and README

## Increments

### Increment 1: Barcode Generator and Reader
- Generate a barcode from a 6-digit number (first digit not 0), show it and save it as an image.
- Read a barcode image and show the 6-digit number.
- The barcode table, encoder and decoder are written by us. OpenCV is used only for images.

### Increment 2: Point of Sale GUI
- The whole system is a GUI (Qt 6 Widgets). Barcode reading is camera based, with a typed box as backup.
- Scanning a barcode shows the product details stored in `Product.txt`.
- The cashier records an order, generates a bill, and the order and bill are saved to text files.
- The manager views barcodes, inventory, adds products and updates price and stock.

## Features

**Main window**
- Cashier and Manager buttons, live date and time, number of products and low stock count.

**Cashier**
- Add Product to Bill: scan with the camera (or type the barcode). The product is added with quantity 1.
- Change the quantity with the up/down arrows or by typing. Count products accept whole numbers, weight products accept up to 2 decimals. A quantity above the available stock is rejected.
- Description: shows the details of the selected product (or asks for a barcode if no row is selected).
- Remove Item, Cancel Order and Back.
- Customer name (letters and spaces only) and contact (optional, exactly 11 digits) are validated.
- Hold Order: saves the current order as `on-hold` and starts a new one. Resume Held Order loads it back later.
- Generate Bill: asks for payment mode, merchant and status, saves the order, order details and bill, reduces stock for paid orders, and shows the bill on screen.

**Manager**
- View Barcode: shows the barcode image of a product (recreated if missing).
- Inventory: table of all products and any skipped lines.
- Add New Product (also creates its barcode image) and Update Price / Quantity.
- Product Scan: scan a barcode to view the product.

## How the Barcode Works
- Each digit has a 7-bit pattern (1 = black bar, 0 = white bar).
- Barcode = start marker (101) + 6 digit patterns + end marker (101).
- Total length: 3 + (6 x 7) + 3 = 48 bits.
- For camera frames, the scanner finds the barcode region, reads several rows and decodes a noisy row. A number is accepted after it is read 3 times in a row.

## Data Files
All files are in the project folder. Fields are separated by a single space, every line ends with `#`, and spaces inside names are written as `_`.

| File | Content |
|------|---------|
| `Product.txt` | id, barcode, acronym (max 8), name, unit (count/weight), unit price, tax, discount, available quantity, barcode image path |
| `Order.txt` | order id (8 digits, last two = day), date, time, customer name, contact (`-` if none), status (in-progress, on-hold, paid, archived), payment amount, payment mode, merchant |
| `OrderDetail.txt` | order id, product id, product barcode, quantity (one line per product) |
| `Bill.txt` | bill id (9 digits), customer name, order id, product acronyms, unit prices, discounts, quantities, total without tax, total tax, total discount, total payable, status |

- In `Bill.txt`, one line is one bill. Acronyms, prices, discounts and quantities are lists separated by `|`, matched by position (for example `MILK|BREAD 280|150 10|0 2|1`).
- Tax and discount are per unit, in PKR.
- Payable = total without tax - total discount + total tax.
- `Order.txt`, `OrderDetail.txt` and `Bill.txt` are created by the program. `barcodes/` holds one image per product (`barcodes/<barcode>.png`).

## Design (SRP and OCP)

| Folder | Role |
|--------|------|
| `barcode/` | Encoder, decoder, drawer and reader from Increment 1 |
| `camera/` | `BarcodeScanner` and `noisycameradecoder`: pixels to number |
| `models/` | Plain data: `Product`, `Order`, `OrderDetail`, `Bill` |
| `data/` | Repositories that read and write the text files, `IdGenerator`, `TextFormat` |
| `services/` | Rules: `BillCalculator` (totals), `OrderService` (cart, stock checks, hold, resume, checkout) |
| `gui/` | Windows and dialogs |

- `IProductRepository` is an interface, so the storage can change without touching the services or windows.
- Services and repositories contain no `cin`, `cout` or Qt, so they can be reused and tested alone.

## Requirements
- C++17 compiler
- Qt 6.5 or newer (Widgets)
- OpenCV 4.12.0 (other 4.x versions should also work)
- CMake 3.16 or newer
- A webcam (for camera scanning)
- Windows: Visual Studio 2022 (or Build Tools) with "Desktop development with C++"

## Build and Run

### Windows
1. Install Qt 6 (MSVC 2022 64-bit kit) and extract OpenCV to `C:\opencv`.
2. Add `C:\opencv\build\x64\vc16\bin` to your PATH and restart the terminal or editor.
3. In the project folder (use your own Qt version in the path):
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR=C:/opencv/build/x64/vc16/lib -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build --config Release
build\Release\store_app.exe
```
4. If a Qt DLL is missing when running, execute `windeployqt build\Release\store_app.exe`.

### macOS
```
xcode-select --install
brew install cmake opencv qt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/store_app
```

**Always run the program from the project folder**, so `Product.txt`, the other data files and `barcodes/` are found.

## Project Structure
```
.
|-- main.cpp              # starts the app, loads products, wires repositories
|-- config.h              # data file names
|-- shared.h              # barcode constants
|-- Product.txt           # sample products
|-- barcodes/             # barcode images of the products
|-- barcode/              # encoder, decoder, drawer, reader
|-- camera/               # barcodescanner, noisycameradecoder
|-- models/               # product, order, orderDetail, bill
|-- data/                 # repositories, IdGenerator, TextFormat
|-- services/             # BillCalculator, OrderService
|-- gui/                  # mainwindow, cashierwindow, managerwindow, inventorywindow,
|                         # scandialog, camerawidget, productdialog, addproductdialog,
|                         # paymentdialog, billdialog, heldordersdialog
|-- CMakeLists.txt
|-- .gitignore
|-- README.md
```

## Testing
| Action | Expected result |
|--------|-----------------|
| Scan a valid product barcode | Product added with quantity 1 |
| Barcode not in `Product.txt` | "No product exists against the barcode ..." |
| Typed barcode `012345`, `12345`, `abc` | Rejected in the scan dialog |
| Quantity above stock | Rejected with a message, old value restored |
| Count product, decimal quantity | Not accepted |
| Weight product, `0.5` | Accepted |
| Customer name `Ali123` | "Invalid name" |
| Contact with 10 digits or letters | "Invalid contact" |
| Generate bill with empty order | Blocked with a message |
| Generate bill (paid) | Three files updated, stock reduced, bill shown |
| Hold order, then Resume | Items, name and contact come back, no stock change while held |
| Resume then Generate Bill | Held record replaced by the paid order |
| Manager: View Barcode `999999` | "Product does not exist" |
| Manager: Add New Product | Line added to `Product.txt`, image created |
| Camera busy or unplugged | Message shown, typed barcode still works |

## Known Limitations
- Camera scanning needs good light and a flat, straight barcode on a plain background. Tilt, glare and blur are not handled.
- Camera access uses the first camera.
- Do not edit the data files while the program is running.