#include <mupdf/fitz.h>
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <sstream>  // To create unique file names
#include <sys/stat.h>  // For stat() and mkdir()
#include <unistd.h> // For access() (optional, if needed for checking file/folder existence)

// Custom callback to write data to a FILE*
void file_write_callback(fz_context* ctx, void* state, const void* data, size_t len) {
    FILE* out_file = static_cast<FILE*>(state);
    fwrite(data, 1, len, out_file);
}

// Custom callback to close the FILE*
void file_close_callback(fz_context* ctx, void* state) {
    FILE* out_file = static_cast<FILE*>(state);
    fclose(out_file);
}

// Custom callback to handle dropping the FILE* (no-op in this case)
void file_drop_callback(fz_context* ctx, void* state) {
    // Nothing to do here, just prevent the resource from being freed automatically
}

void create_directory(const std::string& dir) {
    struct stat st;
    if (stat(dir.c_str(), &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(dir.c_str(), 0777) != 0) {
            std::cerr << "Error creating directory: " << dir << std::endl;
            throw std::runtime_error("Failed to create directory.");
        }
    }
}

void open_and_render_pdf(const char* filename, const char* output_dir) {
    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!ctx) {
        std::cerr << "Failed to create MuPDF context." << std::endl;
        return;
    }

    try {
        fz_try(ctx) {
            fz_register_document_handlers(ctx);

            // Open the document
            fz_document* doc = fz_open_document(ctx, filename);
            if (!doc) {
                throw std::runtime_error("Failed to open document.");
            }

            int page_count = fz_count_pages(ctx, doc);
            std::cout << "Document opened successfully: " << filename << std::endl;
            std::cout << "Number of pages: " << page_count << std::endl;

            // Ensure the output directory exists
            create_directory(output_dir);

            // Iterate over pages and render them
            for (int page_num = 0; page_num < page_count; ++page_num) {
                fz_page* page = fz_load_page(ctx, doc, page_num);

                // Set the page's rendering matrix (for scaling or positioning)
                fz_matrix transform = fz_scale(1.0f, 1.0f); // No scaling, just simple 1:1
                fz_colorspace* cs = fz_device_rgb(ctx);  // Use RGB color space
                int alpha = 0;  // No transparency
                fz_pixmap* pixmap = fz_new_pixmap_from_page(ctx, page, transform, cs, alpha);

                if (!pixmap) {
                    throw std::runtime_error("Failed to create pixmap from page.");
                }

                // Create a unique filename for each page in the specified output directory
                std::ostringstream file_name;
                file_name << output_dir << "/output_page_" << (page_num + 1) << ".png";

                // Open the output file manually using `fopen`
                FILE* out_file = fopen(file_name.str().c_str(), "wb");
                if (!out_file) {
                    throw std::runtime_error("Failed to open output file.");
                }

                // Create an fz_output from the custom callbacks
                fz_output* out = fz_new_output(ctx, 1024, out_file, file_write_callback, file_close_callback, file_drop_callback);

                // Write the pixmap as PNG to the output stream
                std::cout << "Writing page " << (page_num + 1) << " to " << file_name.str() << std::endl;
                fz_write_pixmap_as_png(ctx, out, pixmap);

                // Close the output file (already handled by callback)
                fclose(out_file);

                // Drop the pixmap and page to free resources
                fz_drop_pixmap(ctx, pixmap);
                fz_drop_page(ctx, page);
            }

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
    const char* output_dir = "./output_images";  // Path to the folder for saving PNG files relative to the project root
    open_and_render_pdf(filename, output_dir);
    return 0;
}
