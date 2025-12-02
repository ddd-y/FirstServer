#pragma once


#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <Eigen/Dense>
#include <Eigen/Sparse>


class UnifiedData {
public:
    enum class DataType { JSON, MATRIX_DENSE, MATRIX_SPARSE, TEXT };

private:
    DataType type_;
    std::variant<
        std::string,
        Eigen::MatrixXd,
        Eigen::SparseMatrix<double>
    > data_;
    std::string metadata_;

public:
    // 构造函数
    UnifiedData(const std::string& json_data)
        : type_(DataType::JSON), data_(json_data) {}

    UnifiedData(const Eigen::MatrixXd& matrix)
        : type_(DataType::MATRIX_DENSE), data_(matrix) {}

    UnifiedData(const Eigen::SparseMatrix<double>& matrix)
        : type_(DataType::MATRIX_SPARSE), data_(matrix) {}

    UnifiedData(const char* text)
        : type_(DataType::TEXT), data_(std::string(text)) {}

  
    DataType getType() const { return type_; }
    bool isJson() const { return type_ == DataType::JSON; }
    bool isMatrixDense() const { return type_ == DataType::MATRIX_DENSE; }
    bool isMatrixSparse() const { return type_ == DataType::MATRIX_SPARSE; }
    bool isMatrix() const { return isMatrixDense() || isMatrixSparse(); }
    bool isText() const { return type_ == DataType::TEXT; }

  
    const std::string& getJson() const { return std::get<std::string>(data_); }
    const Eigen::MatrixXd& getMatrixDense() const { return std::get<Eigen::MatrixXd>(data_); }
    const Eigen::SparseMatrix<double>& getMatrixSparse() const { return std::get<Eigen::SparseMatrix<double>>(data_); }
    const std::string& getText() const { return std::get<std::string>(data_); }

 
    int getMatrixRows() const {
        if (isMatrixDense()) return getMatrixDense().rows();
        if (isMatrixSparse()) return getMatrixSparse().rows();
        return 0;
    }

    int getMatrixCols() const {
        if (isMatrixDense()) return getMatrixDense().cols();
        if (isMatrixSparse()) return getMatrixSparse().cols();
        return 0;
    }

    // 元数据
    void setMetadata(const std::string& metadata) { metadata_ = metadata; }
    const std::string& getMetadata() const { return metadata_; }
};

