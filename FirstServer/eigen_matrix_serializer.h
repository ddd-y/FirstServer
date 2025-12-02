#pragma once
#include"unified_data.h"


class EigenMatrixSerializer {
public:
    // 序列化稠密矩阵
    static std::vector<uint8_t> serializeDense(const Eigen::MatrixXd& matrix, bool compress = false) {
        if (matrix.size() == 0) return {};

        // 基础序列化
        std::vector<uint8_t> buffer = serializeDenseBasic(matrix);

        // 可选压缩
        if (compress) {
            buffer = compressData(buffer);
        }

        return buffer;
    }

    // 序列化稀疏矩阵
    static std::vector<uint8_t> serializeSparse(const Eigen::SparseMatrix<double>& matrix, bool compress = false) {
        if (matrix.nonZeros() == 0) return {};

        // 基础序列化
        std::vector<uint8_t> buffer = serializeSparseBasic(matrix);

        // 可选压缩
        if (compress) {
            buffer = compressData(buffer);
        }

        return buffer;
    }

    // 通用序列化接口
    static std::vector<uint8_t> serialize(const UnifiedData& data, bool compress = false) {
        if (data.isMatrixDense()) {
            return serializeDense(data.getMatrixDense(), compress);
        }
        else if (data.isMatrixSparse()) {
            return serializeSparse(data.getMatrixSparse(), compress);
        }
        return {};
    }

    // 反序列化稠密矩阵
    static Eigen::MatrixXd deserializeDense(const std::vector<uint8_t>& buffer, bool compressed = false) {
        if (buffer.empty()) return Eigen::MatrixXd();

        std::vector<uint8_t> data = buffer;
        if (compressed) {
            data = decompressData(data);
        }

        return deserializeDenseBasic(data);
    }

    // 反序列化稀疏矩阵
    static Eigen::SparseMatrix<double> deserializeSparse(const std::vector<uint8_t>& buffer, bool compressed = false) {
        if (buffer.empty()) return Eigen::SparseMatrix<double>();

        std::vector<uint8_t> data = buffer;
        if (compressed) {
            data = decompressData(data);
        }

        return deserializeSparseBasic(data);
    }

private:

    struct DenseMatrixHeader {
        int rows;
        int cols;
        int storage_order; // 0=列优先, 1=行优先
        size_t data_size;
    };

    struct SparseMatrixHeader {
        int rows;
        int cols;
        int non_zeros;
        size_t indices_size;
        size_t values_size;
    };

    // 稠密矩阵基础序列化
    static std::vector<uint8_t> serializeDenseBasic(const Eigen::MatrixXd& matrix) {
        // 序列化矩阵头信息
        

        DenseMatrixHeader header = {
            matrix.rows(),
            matrix.cols(),
            (matrix.IsRowMajor ? 1 : 0),
            matrix.size() * sizeof(double)
        };

        std::vector<uint8_t> buffer(sizeof(DenseMatrixHeader) + header.data_size);
        memcpy(buffer.data(), &header, sizeof(DenseMatrixHeader));

        // 直接拷贝数据（Eigen矩阵在内存中是连续的）
        memcpy(buffer.data() + sizeof(DenseMatrixHeader),
            matrix.data(), header.data_size);

        return buffer;
    }

    // 稀疏矩阵基础序列化（使用COO格式）
    static std::vector<uint8_t> serializeSparseBasic(const Eigen::SparseMatrix<double>& matrix) {
        // 序列化稀疏矩阵头信息
        

        SparseMatrixHeader header = {
            matrix.rows(),
            matrix.cols(),
            static_cast<int>(matrix.nonZeros()),
            matrix.nonZeros() * sizeof(int),
            matrix.nonZeros() * sizeof(double)
        };

        // 转换为COO格式进行序列化
        std::vector<int> row_indices;
        std::vector<int> col_indices;
        std::vector<double> values;

        for (int k = 0; k < matrix.outerSize(); ++k) {
            for (Eigen::SparseMatrix<double>::InnerIterator it(matrix, k); it; ++it) {
                row_indices.push_back(it.row());
                col_indices.push_back(it.col());
                values.push_back(it.value());
            }
        }

        size_t total_size = sizeof(SparseMatrixHeader) +
            header.indices_size * 2 +  // row和col索引
            header.values_size;

        std::vector<uint8_t> buffer(total_size);
        size_t offset = 0;

        // 写入头信息
        memcpy(buffer.data() + offset, &header, sizeof(header));
        offset += sizeof(header);

        // 写入行索引
        memcpy(buffer.data() + offset, row_indices.data(), header.indices_size);
        offset += header.indices_size;

        // 写入列索引
        memcpy(buffer.data() + offset, col_indices.data(), header.indices_size);
        offset += header.indices_size;

        // 写入数值
        memcpy(buffer.data() + offset, values.data(), header.values_size);

        return buffer;
    }

    // 稠密矩阵基础反序列化
    static Eigen::MatrixXd deserializeDenseBasic(const std::vector<uint8_t>& data) {




        if (data.size() < sizeof(struct DenseMatrixHeader)) {
            return Eigen::MatrixXd();
        }

        DenseMatrixHeader header;
        memcpy(&header, data.data(), sizeof(header));

        // 创建Eigen矩阵
        Eigen::MatrixXd matrix(header.rows, header.cols);

        if (header.data_size == matrix.size() * sizeof(double)) {
            memcpy(matrix.data(), data.data() + sizeof(header), header.data_size);
        }

        return matrix;
    }

    // 稀疏矩阵基础反序列化
    static Eigen::SparseMatrix<double> deserializeSparseBasic(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(struct SparseMatrixHeader)) {
            return Eigen::SparseMatrix<double>();
        }

        SparseMatrixHeader header;
        memcpy(&header, data.data(), sizeof(header));

        size_t offset = sizeof(header);

        // 读取行索引
        std::vector<int> row_indices(header.non_zeros);
        memcpy(row_indices.data(), data.data() + offset, header.indices_size);
        offset += header.indices_size;

        // 读取列索引
        std::vector<int> col_indices(header.non_zeros);
        memcpy(col_indices.data(), data.data() + offset, header.indices_size);
        offset += header.indices_size;

        // 读取数值
        std::vector<double> values(header.non_zeros);
        memcpy(values.data(), data.data() + offset, header.values_size);

        // 构建稀疏矩阵
        Eigen::SparseMatrix<double> matrix(header.rows, header.cols);
        std::vector<Eigen::Triplet<double>> triplets;

        for (int i = 0; i < header.non_zeros; ++i) {
            triplets.emplace_back(row_indices[i], col_indices[i], values[i]);
        }

        matrix.setFromTriplets(triplets.begin(), triplets.end());
        return matrix;
    }

    static std::vector<uint8_t> compressData(const std::vector<uint8_t>& data) {
        // 使用zlib或类似库进行压缩
        // 这里简化为返回原数据，实际应实现压缩
        return data;
    }

    static std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data) {
        // 解压缩实现
        return data;
    }
};