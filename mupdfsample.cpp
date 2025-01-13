#include <mupdf/fitz.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <openssl/evp.h> // Include OpenSSL for Base64 encoding
#include <nlohmann/json.hpp> // Include the JSON library

using json = nlohmann::json; // Alias for convenience

// Function to encode data to Base64
std::string base64_encode(const unsigned char* data, size_t input_length) {
    if (input_length == 0) return "";

    // Calculate the output length
    int output_length = 4 * ((input_length + 2) / 3);
    std::string encoded_data(output_length, '\0');

    // Perform Base64 encoding
    EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&encoded_data[0]), data, input_length);
    return encoded_data;
}

// Class to handle PDF rendering
class PdfRenderer {
public:
    PdfRenderer(const std::string& filename);
    ~PdfRenderer();

    std::vector<unsigned char> renderPage(int page_num); // Method to render a specific page and return raw bytes
    int getTotalPages(); // Method to get the total number of pages
    json getMetadata(); // Method to get document metadata as JSON

private:
    fz_context* ctx;      // MuPDF context
    fz_document* doc;     // MuPDF document
    std::string filename;  // Path to the PDF file
};

// Constructor: Initializes MuPDF context and opens the document
PdfRenderer::PdfRenderer(const std::string& filename) : filename(filename), ctx(nullptr), doc(nullptr) {
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!ctx) {
        std::cerr << "Failed to create MuPDF context." << std::endl;
        return;
    }

    fz_register_document_handlers(ctx);

    // Open the PDF document
    fz_try(ctx) {
        doc = fz_open_document(ctx, filename.c_str());
    }
    fz_catch(ctx) {
        std::cerr << "Failed to open document: " << filename << std::endl;
        std::cerr << "MuPDF error: " << fz_caught_message(ctx) << std::endl;
        fz_drop_context(ctx);
        ctx = nullptr;
    }
}

// Destructor: Cleans up resources
PdfRenderer::~PdfRenderer() {
    if (doc) {
        fz_drop_document(ctx, doc);
    }
    if (ctx) {
        fz_drop_context(ctx);
    }
}

// Method to render a specific page and return raw bytes
std::vector<unsigned char> PdfRenderer::renderPage(int page_num) {
    int total_pages = fz_count_pages(ctx, doc);
    if (page_num < 0 || page_num >= total_pages) {
        std::cerr << "Invalid page number: " << page_num << std::endl;
        return {}; // Return an empty vector if the page number is invalid
    }

    // Proceed with rendering the page
    fz_page* page = fz_load_page(ctx, doc, page_num);
    fz_matrix transform = fz_scale(1.0f, 1.0f); // No scaling
    fz_colorspace* cs = fz_device_rgb(ctx);
    int alpha = 0;

    fz_pixmap* pixmap = fz_new_pixmap_from_page(ctx, page, transform, cs, alpha);
    if (!pixmap) {
        std::cerr << "Failed to create pixmap for page: " << page_num << std::endl;
        fz_drop_page(ctx, page);
        return {}; // Handle the error appropriately
    }

    // Convert pixmap to raw bytes
    size_t image_size = pixmap->w * pixmap->h * 4; // Assuming RGBA
    std::vector<unsigned char> image_data(image_size);
    std::memcpy(image_data.data(), pixmap->samples, image_size); // Copy raw bytes

    fz_drop_pixmap(ctx, pixmap); // Clean up the pixmap
    fz_drop_page(ctx, page); // Clean up the loaded page

    return image_data; // Return the raw image bytes
}

// Method to get the total number of pages
int PdfRenderer::getTotalPages() {
    if (!doc) {
        std::cerr << "Document not opened." << std::endl;
        return 0; // Return 0 if document is not opened
    }
    return fz_count_pages(ctx, doc); // Pass ctx along with doc
}

// Method to get document metadata as JSON
json PdfRenderer::getMetadata() {
    json metadata_json;

    if (!doc) {
        std::cerr << "Document not opened." << std::endl;
        return metadata_json; // Return empty JSON if document is not opened
    }

    char metadata_buffer[256]; // Buffer to hold metadata strings

    // Retrieve and populate various metadata fields
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_TITLE, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["title"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_AUTHOR, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["author"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_SUBJECT, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["subject"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_KEYWORDS, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["keywords"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_CREATOR, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["creator"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_PRODUCER, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["producer"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_CREATIONDATE, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["creation_date"] = metadata_buffer;
    if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_MODIFICATIONDATE, metadata_buffer, sizeof(metadata_buffer)) > 0)
        metadata_json["modification_date"] = metadata_buffer;

    return metadata_json; // Return the JSON object containing metadata
}

// Example usage
int main() {
    std::string filename = "/home/prodata/Downloads/o_poder_do_subconsciente.pdf"; // Path to the PDF file
    PdfRenderer renderer(filename);

    // Get and print document metadata as JSON
    json metadata = renderer.getMetadata(); // Get metadata for the document
    std::cout << "Document Metadata: " << metadata.dump(4) << std::endl; // Pretty print JSON with 4 spaces

    // Get and print all pages images as Base64
    std::vector<std::string> all_images; // Store Base64 images
    int total_pages = renderer.getTotalPages(); // Get total pages
    for (int i = 0; i < total_pages; ++i) {
        std::vector<unsigned char> image_data = renderer.renderPage(i); // Render each page
        if (!image_data.empty()) {
            all_images.push_back(base64_encode(image_data.data(), image_data.size())); // Encode to Base64
        }
    }

    json images_json = json::array();
    for (size_t i = 0; i < 1; ++i) {
        images_json.push_back({{"page" + std::to_string(i + 1), all_images[i]}}); // Use "page1", "page2", etc.
    }
    std::cout << "All Pages Images: " << images_json.dump(4) << std::endl; // Pretty print JSON with 4 spaces

    return 0;
}

