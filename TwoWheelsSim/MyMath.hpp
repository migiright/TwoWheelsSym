#pragma once

#include <array>
#include <boost/format.hpp>
#include <boost/operators.hpp>
#include <boost/range.hpp>

/**
* @brief 行列
* @tparam H 行数
* @tparam W 列数
*/
template<size_t H, size_t W>
class Matrix
	: boost::multipliable<Matrix<H, W>>
{
public:
	constexpr static size_t Height = H; ///< 行数
	constexpr static size_t Width = W; ///< 列数

	std::array<double, Height*Width> v;

	/// 0で初期化する
	Matrix()
		: v{}
	{}

	/// コピーコンストラクタ
	Matrix(const Matrix<H, W> &o)
		: v(o.v)
	{}

	/**
	* \brief 初期値を指定する
	* \param args 初期化する値。1行1列, ... , 1行Width列, 2行1列, ... の順
	*/
	template<class ...Args>
	explicit Matrix(const Args &...args)
		: v{args...}
	{}

	/// 要素を返す
	double& operator()(size_t row, size_t column)
	{
		return v[row * Width + column];
	}

	/// 要素を返す
	double operator()(size_t row, size_t column) const
	{
		return v[row * Width + column];
	}

	constexpr static double height() { return Height; } ///< 行数を返す
	constexpr static double width() { return Width; } ///< 列数を返す

													  /// 整形して文字列として返す
	std::string toString() const
	{
		std::string s;
		for (size_t r = 0; r < Height; ++r) {
			for (size_t c = 0; c < Width; ++c) {
				s += (boost::format("%4.2f") % (*this)(r, c)).str() + " ";
			}
			s += "\n";
		}
		return s;
	}

	/// スカラとの乗算
	Matrix<Height, Width>& operator*=(double rhs)
	{
		for (size_t r = 0; r < Height; ++r) {
			for (size_t c = 0; c < Width; ++c) {
				(*this)(r, c) *= rhs;
			}
		}
		return *this;
	}

};

/// 行列同士の乗算
template<size_t LhsWidth, size_t AnsHright, size_t AnsWidth>
Matrix<AnsHright, AnsWidth> operator*(
	const Matrix<AnsHright, LhsWidth>& lhs, const Matrix<LhsWidth, AnsWidth>& rhs)
{
	Matrix<AnsHright, AnsWidth> ret;
	for (size_t r = 0; r < AnsHright; ++r) {
		for (size_t c = 0; c < AnsWidth; ++c) {
			double s = 0;
			for (size_t i = 0; i < LhsWidth; ++i) {
				s += lhs(r, i) * rhs(i, c);
			}
			ret(r, c) = s;
		}
	}
	return ret;
}


/**
* \brief ベクトル
* \tparam D 次元
*/
template<size_t D>
struct Vector
	: boost::addable<Vector<D>>
	, boost::subtractable<Vector<D>>
	, boost::multipliable<Vector<D>, double>
	, boost::dividable<Vector<D>, double>
{
	std::array<double, D> v;
	constexpr static size_t Dimension = D;

	/// 0で初期化する
	Vector() : v{}
	{}

	/// コピーコンストラクタ
	Vector(const Vector &o)
		: v(o.v)
	{}

	/// 初期値を指定する
	template<class ...Args>
	explicit Vector(const Args &...args)
		: v{args...}
	{}

	/// 要素を返す
	double& operator[](unsigned index) { return v[index]; }

	/// 要素を返す
	const double& operator[](unsigned index) const { return v[index]; }

	/// 加算
	Vector<D>& operator+=(const Vector<D>& rhs)
	{
		for (size_t i = 0; i < D; i++) {
			v[i] += rhs[i];
		}
		return *this;
	}

	/// 減算
	Vector<D>& operator-=(const Vector<D>& rhs)
	{
		for (size_t i = 0; i < D; i++) {
			v[i] -= rhs[i];
		}
		return *this;
	}

	/// スカラとの乗算
	Vector<D>& operator*=(double rhs)
	{
		for (size_t i = 0; i < D; ++i) {
			(*this)[i] *= rhs;
		}
		return *this;
	}

	/// スカラでの除算
	Vector<D>& operator/=(double rhs)
	{
		for (size_t i = 0; i < D; ++i) {
			(*this)[i] /= rhs;
		}
		return *this;
	}

	/// 整形して文字列として返す
	std::string toString()
	{
		std::string s;
		for (size_t i = 0; i < D; ++i) {
			s += (boost::format("%4.2f") % (*this)[i]).str() + " ";
		}
		s += "\n";
		return s;
	}
};

/// 行列*縦ベクトルの乗算
template<size_t LhsHeight, size_t LhsWidth>
Vector<LhsHeight> operator*(Matrix<LhsHeight, LhsWidth> lhs, Vector<LhsWidth> rhs)
{
	Vector<LhsHeight> ret;
	for (size_t i = 0; i < LhsHeight; i++) {
		ret[i] = 0;
		for (size_t j = 0; j < LhsWidth; j++) {
			ret[i] += lhs(i, j) * rhs[j];
		}
	}
	return ret;
}

/// 横ベクトル*行列の乗算
template<size_t LhsHeight, size_t LhsWidth>
Vector<LhsWidth> operator*(Vector<LhsHeight> rhs, Matrix<LhsHeight, LhsWidth> lhs)
{
	Vector<LhsWitdh> ret;
	for (size_t i = 0; i < LhsWidth; i++) {
		ret[i] = 0;
		for (size_t j = 0; j < LhsHeight; j++) {
			ret[i] += rhs[j] * lhs(j, i);
		}
	}
	return ret;
}
