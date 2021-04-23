#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>

#include "class.h"

using namespace std;

#define PALETTE_LENGTH_BYTES 54

/* Takes text from the TEXT string, covers it in INPUT file
 * with compression DEGREE bites per byte.
 * Make sure that degree is 1, 2, 4 or 8
 * and check if global variable PALETTE_LENGTH_BYTES matches with length in your file. */
int package(const string& input, const string& inputText, const unsigned degree){
    if (degree != 1 && degree != 2 && degree != 4 && degree != 8){
        cout << "Sorry, wrong degree :(" << endl;
        return -1;
    }

    ifstream image(input, std::ios::binary);
    ifstream message(inputText);

    if(image.fail() || message.fail()) {
        cout << "File was not found :(\n";
        return -1;
    }

    message.seekg(0, std::ifstream::end);
    long int textInBytes = message.tellg();
    message.seekg(0, std::ifstream::beg);

    image.seekg(0, std::ifstream::end);
    long int imageInBytes = image.tellg();
    image.seekg(0, std::ifstream::beg);

    long int needed = 3 /* length in bytes.*/ +
                        4 * (8 / degree) /* expansion in bytes. */ +
                        textInBytes * (8 / degree) /* text in bytes. */ +
                        2 * PALETTE_LENGTH_BYTES /* For palette, on both sides of the picture. */;

    /* Check that container has enough space to cover message. */
    if(imageInBytes < needed){
        cout << "Not enough space in picture.\n" << "Image has " << imageInBytes
        << " bytes, but needed " << needed << " bytes to hide text.\n";
        return -1;
    } else cout << "Enough space in picture.\n" << "Image has " << imageInBytes
    << " bytes, and needed " << needed << " bytes to hide text.\n" << "Text length is " << textInBytes << ".\n";

    unsigned char c = 0;
    unsigned char t = 0;
    const string imageExpansion = input.substr(input.rfind('.'), string::npos);
    const string name = input.substr(0, input.rfind('.'));
    const string copy = "Copy";

    string textExpansion = inputText.substr(inputText.rfind('.') + 1, string::npos);

    /* Output stream. */
    ofstream output((name + copy + imageExpansion).c_str(), std::ios::binary);
    char buffer[PALETTE_LENGTH_BYTES];

    /* Skips first PALETTE_LENGTH_BYTES bytes in file, which should not be changed. */
    image.read(buffer, PALETTE_LENGTH_BYTES);
    for(char i : buffer)
        output << i;

    /* Firstly in file will be hidden the length of the hidden message. This value will be covered in 24 bites.
     * In this case the maximum length of the message will be 2 in power 24, which is 16 777 215 symbols. */

    numberWithBites textLength;
    textLength.resize(24);
    textLength.setValue(textInBytes);
    numberWithBites textExpansionSymbol;
    numberWithBites imageByte;

    /* Coding text length. */
    for(int i = 0; i < (24 / degree); i++){
        image.read(reinterpret_cast<char *>(&c), 1);
        imageByte.setValue(c);

        vector <unsigned> bites = textLength.giveBites(degree);
        imageByte.putBites(bites, degree);

        output << char (imageByte.giveNumber());
    }

    /* Coding text expansion. */
    for(int i = 0; i < 4; i++){
        if(i < textExpansion.length())
            textExpansionSymbol.setValue(textExpansion[i]);
        else
            textExpansionSymbol.setValue(0);

        for(int j = 0; j < (8 / degree); j++){
            image.read(reinterpret_cast<char *>(&c), 1);
            imageByte.setValue(c);

            vector <unsigned> bites = textExpansionSymbol.giveBites(degree);
            imageByte.putBites(bites, degree);

            output << char (imageByte.giveNumber());
        }
    }

    /* Coding text. */
    while(!message.eof()){
        message.read(reinterpret_cast<char *>(&t), 1);
        numberWithBites textByte;
        textByte.setValue(t);
        for(int j = 0; j < (8 / degree); j++){
            image.read(reinterpret_cast<char *>(&c), 1);
            imageByte.setValue(c);

            vector <unsigned> bites = textByte.giveBites(degree);
            imageByte.putBites(bites, degree);

            unsigned imageChanged = imageByte.giveNumber();
            output << char (imageChanged);
        }
    }

    /* Copies last part of the file. */
    std::copy(istreambuf_iterator<char>(image),istreambuf_iterator<char>( ), std::ostreambuf_iterator<char>(output));

    image.close();
    output.close();

    if(remove(input.c_str())){
        cout << "Error with deleting initial file.\n";
        return -1;
    };

    if(rename((name + copy + imageExpansion).c_str(), input.c_str())){
        cout << "Error with renaming new file.\n";
        return -1;
    };
    return 0;
}

/* Decodes text length, filename expansion and text by itself from INPUT image file
 * and writes text in new file, named DECODEDTEXT + expansion. */
int unpackage(const string& input, const unsigned degree){
    if (degree != 1 && degree != 2 && degree != 4 && degree != 8){
        cout << "Sorry, wrong degree :(" << endl;
        return -1;
    }

    ifstream image(input, std::ios::binary);

    if(image.fail()){
        cout << "File was not found :(\n";
        return -1;
    }

    /* Skips first PALETTE_LENGTH_BYTES bytes in file, which should not be changed. */
    image.seekg(PALETTE_LENGTH_BYTES, std::ifstream::beg);

    numberWithBites textLength;
    textLength.resize(24);
    numberWithBites imageByte;
    numberWithBites textByte;
    unsigned char c = 0;

    for(int i = 0; i < (24 / degree); i++){
        image.read(reinterpret_cast<char *>(&c), 1);
        imageByte.setValue(c);

        vector <unsigned> bites = imageByte.giveBites(degree);
        textLength.putBites(bites, degree);
    }

    unsigned length = textLength.giveNumber();
    string textExpansion = "0000";

    for(int i = 0; i < 4; i++){
        textByte.setValue(0);
        for(int j = 0; j < (8 / degree); j++){
            image.read(reinterpret_cast<char *>(&c), 1);
            imageByte.setValue(c);

            vector <unsigned> bites = imageByte.giveBites(degree);
            textByte.putBites(bites, degree);
        }
        textExpansion[i] = char(textByte.giveNumber());
    }

    ofstream output (("decodedText." + textExpansion).c_str(), std::ios::binary);
    for(int i = 0; i < length; i++){
        textByte.setValue(0);
        for(int j = 0; j < (8 / degree); j++) {
            image.read(reinterpret_cast<char *>(&c), 1);
            imageByte.setValue(c);

            vector<unsigned> bites = imageByte.giveBites(degree);
            textByte.putBites(bites, degree);
        }
        output << char (textByte.giveNumber());
    }
    output.close();
    image.close();
    return 0;
}
int main() {
    const string input = "Untitled.bmp";

    const string inputText = "Text.txt";
    const unsigned degree = 5;

    package(input, inputText, degree);
    unpackage(input, degree);
    return 0;
}
