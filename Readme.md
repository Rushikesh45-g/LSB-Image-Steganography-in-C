
# Image Steganography using LSB Technique with Magic String (C)

## Project Overview

This project implements **Image Steganography** using the **Least Significant Bit (LSB)** technique in **C programming**. It securely hides secret text data inside a **BMP image**.
To ensure **data authenticity and correct decoding**, a **Magic String** is embedded along with the secret data.

The Magic String acts as a **unique identifier** to confirm the presence of hidden data in the stego image before decoding.

---

## Objectives

* To implement LSB-based image steganography
* To hide secret text data inside a BMP image
* To use a **Magic String for validation**
* To safely extract hidden data only from valid stego images
* To strengthen understanding of bitwise operations and file handling

---

## Magic String Concept

* A **Magic String** is a predefined constant string (e.g., `"STEG"`).
* It is embedded **before the secret data** during encoding.
* During decoding, the extracted magic string is verified.
* If the magic string matches, decoding continues.
* If it does not match, the image is declared **not a valid stego image**.

 This prevents accidental or invalid decoding.

---

## Technology Used

* **Language:** C
* **Image Format:** BMP
* **Techniques:**

  * Least Significant Bit (LSB)
  * Magic String validation
  * File I/O
  * Bitwise manipulation

---

## Project Structure

```
lsb_steganography/
│
├── main.c
├── encode.c
├── decode.c
├── encode.h
├── decode.h
├── types.h
├── common.h
├── sample.bmp
├── stego.bmp
├── secret.txt
└── README.md
```

---

##  Encoding Process

1. Read source BMP image.
2. Read secret text file.
3. Embed the **Magic String** into the image.
4. Embed secret file size and secret data using LSB.
5. Generate stego image.

### Encoding Command

```bash
./a.out -e sample.bmp secret.txt stego.bmp
```

---

##  Decoding Process

1. Read stego image.
2. Extract and verify the **Magic String**.
3. If valid, extract secret file size and data.
4. Save extracted data to output file.

### Decoding Command

```bash
./a.out -d stego.bmp decoded.txt
```

---

## Features

* Magic String based validation
* Secure data extraction
* Lossless image quality
* Modular and readable C code
* Command-line execution

---

# Author

**Rushikesh Ghuge**
Electronics & Telecommunication Engineer

---

