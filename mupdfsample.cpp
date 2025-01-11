#include <mupdf/fitz.h>
#include <iostream>
#include <stdexcept>

void open_pdf_document(const char* filename) {

    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!ctx) {
        std::cerr << "Failed to create MuPDF context." << std::endl;
        return;
    }

    try {

        fz_try(ctx) {
            fz_register_document_handlers(ctx);


            fz_document* doc = fz_open_document(ctx, filename);
            if (!doc) {
                throw std::runtime_error("Failed to open document.");
            }


            int page_count = fz_count_pages(ctx, doc);
            std::cout << "Document opened successfully: " << filename << std::endl;
            std::cout << "Number of pages: " << page_count << std::endl;


            fz_drop_document(ctx, doc);
        }
        fz_catch(ctx) {

            std::cerr << "MuPDF error: " << fz_caught_message(ctx) << std::endl;
            throw std::runtime_error("Error in MuPDF library.");
        }
    } catch (const std::exception& e) {
        // Catch standard exceptions
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Clean up the MuPDF context
    fz_drop_context(ctx);
}

int main() {
    const char* filename = "/home/prodata/Downloads/CV_Erickson.pdf"; // Path to the input PDF file
    open_pdf_document(filename);
    return 0;
}


