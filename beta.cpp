// string hashStringToSHA256(const std::string& input) {
//     // Create a buffer to hold the SHA-256 hash
//     unsigned char hash[SHA256_DIGEST_LENGTH];
    
//     // Compute the SHA-256 hash
//     SHA256_CTX sha256;
//     SHA256_Init(&sha256);
//     SHA256_Update(&sha256, input.c_str(), input.size());
//     SHA256_Final(hash, &sha256);
    
//     // Convert the hash to a hex string
//     std::ostringstream hexStream;
//     for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//         hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
//     }
    
//     return hexStream.str();
// }