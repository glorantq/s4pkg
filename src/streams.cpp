#include <s4pkg/internal/streams.h>

#include <s4pkg/packageexception.h>

namespace s4pkg::internal::streams {

void readBytes(std::istream& stream, uint8_t* buffer, int size) {
    for (int i = 0; i < size; i++) {
        if (!stream.good()) {
            throw PackageException("Unexpected end of stream!");
        }

        stream.read((char*)(buffer + i), 1);
    }
}

void readUint32(std::istream& stream, uint32_t& value) {
    uint8_t buffer[4];
    readBytes(stream, buffer, 4);

    value = ((uint32_t)buffer[3] << 24 | (uint32_t)buffer[2] << 16 |
             (uint32_t)buffer[1] << 8 | (uint32_t)buffer[0]);
}

void readInt32(std::istream& stream, int32_t& value) {
    uint32_t temp;
    readUint32(stream, temp);

    value = (int32_t)temp;
}

void readUint64(std::istream& stream, uint64_t& value) {
    uint8_t buffer[8];
    readBytes(stream, buffer, 8);
    value = ((uint64_t)buffer[7] << 56 | (uint64_t)buffer[6] << 48 |
             (uint64_t)buffer[5] << 40 | (uint64_t)buffer[4] << 32 |
             (uint64_t)buffer[3] << 24 | (uint64_t)buffer[2] << 16 |
             (uint64_t)buffer[1] << 8 | (uint64_t)buffer[0]);
}

void readUint32Array(std::istream& stream, uint32_t* buffer, int size) {
    for (int i = 0; i < size; i++) {
        readUint32(stream, buffer[i]);
    }
}

void readPackageTime(std::istream& stream, package_time_t& value) {
    readInt32(stream, value);
}

void readPackageVersion(std::istream& stream, package_version_t& value) {
    readUint32(stream, value.m_major);
    readUint32(stream, value.m_minor);
}

void readPackageHeader(std::istream& stream, package_header_t& value) {
    readBytes(stream, value.m_fileIdentifier, 4);

    uint8_t expectedIdentifier[] = {'D', 'B', 'P', 'F'};

    for (int i = 0; i < 4; i++) {
        if (value.m_fileIdentifier[i] != expectedIdentifier[i]) {
            throw PackageException("Invalid file identifier!");
        }
    }

    readPackageVersion(stream, value.m_fileVersion);
    readPackageVersion(stream, value.m_userVersion);

    readUint32(stream, value.m_unused1);

    readPackageTime(stream, value.m_creationTime);
    readPackageTime(stream, value.m_updatedTime);

    readUint32(stream, value.m_unused2);

    readUint32(stream, value.m_indexRecordEntryCount);
    readUint32(stream, value.m_indexRecordPositionLow);
    readUint32(stream, value.m_indexRecordSize);

    readUint32Array(stream, value.m_unused3, 3);
    readUint32(stream, value.m_unused4);

    readUint64(stream, value.m_indexRecordPosition);

    readUint32Array(stream, value.m_unused5, 6);
}

}  // namespace s4pkg::internal::streams
