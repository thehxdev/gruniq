namespace gruniq::io {

std::streamoff read_file(const char *path, std::string &out_buffer) {
    std::ifstream fs{path};
    if (!fs.is_open())
        return 0;

    fs.seekg(0, std::ios::end);
    std::streamoff file_size{fs.tellg()};
    out_buffer.resize(file_size);
    fs.seekg(0);

    fs.read(out_buffer.data(), out_buffer.size());
    fs.close();

    return file_size;
}

} // end namespace gruniq::io
