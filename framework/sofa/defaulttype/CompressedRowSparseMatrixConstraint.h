/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 MGH, INRIA, USTL, UJF, CNRS, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_DEFAULTTYPE_COMPRESSEDROWSPARSEMATRIXCONSTRAINT_H
#define SOFA_DEFAULTTYPE_COMPRESSEDROWSPARSEMATRIXCONSTRAINT_H

#include <sofa/defaulttype/CompressedRowSparseMatrix.h>
#include <sofa/SofaFramework.h>

namespace sofa
{

namespace defaulttype
{

template<class RowType, class VecDeriv, typename Real = typename VecDeriv::value_type::Real>
Real CompressedRowSparseMatrixVecDerivMult(const RowType row, const VecDeriv& vec)
{
    Real r = 0;
    for (typename RowType::first_type it = row.first, itend = row.second; it != itend; ++it)
        r += it.val() * vec[it.index()];
    return r;
}

template<class RowType, class VecDeriv>
void convertCompressedRowSparseMatrixRowToVecDeriv(const RowType row, VecDeriv& out)
{
    for (typename RowType::first_type it = row.first, itend = row.second; it != itend; ++it)
    {
        out[it.index()] += it.val();
    }
}

/// Constraint policy type, showing the types and flags to give to CompressedRowSparseMatrix
/// for its second template type. The default values correspond to the original implementation.
class CRSConstraintPolicy : public CRSDefaultPolicy
{
public:
    static constexpr bool AutoSize = true;
    static constexpr bool AutoCompress = true;
    static constexpr bool CompressZeros = false;
    static constexpr bool ClearByZeros = false;
    static constexpr bool OrderedInsertion = false;

    static constexpr int  matrixType = 2;
};

template<typename TBloc, typename TPolicy = CRSConstraintPolicy >
class CompressedRowSparseMatrixConstraint : public sofa::defaulttype::CompressedRowSparseMatrix<TBloc, TPolicy>
{
public:
    typedef CompressedRowSparseMatrixConstraint<TBloc, TPolicy> Matrix;
    typedef CompressedRowSparseMatrix<TBloc, TPolicy> CRSMatrix;
    typedef typename CRSMatrix::Policy Policy;

    using Bloc     = TBloc;
    using VecBloc  = typename CRSBlocTraits<Bloc>::VecBloc;
    using VecIndex = typename CRSBlocTraits<Bloc>::VecIndex;
    using VecFlag  = typename CRSBlocTraits<Bloc>::VecFlag;
    using Index    = typename VecIndex::value_type;

    typedef typename CRSMatrix::Bloc Data;
    typedef typename CRSMatrix::Range Range;
    typedef typename CRSMatrix::traits traits;
    typedef typename CRSMatrix::Real Real;
    typedef typename CRSMatrix::Index KeyType;
    typedef typename CRSMatrix::IndexedBloc IndexedBloc;

public:
    CompressedRowSparseMatrixConstraint()
        : CRSMatrix()
    {
    }

    CompressedRowSparseMatrixConstraint(Index nbRow, Index nbCol)
        : CRSMatrix(nbRow, nbCol)
    {
    }

    bool empty() const
    {
        return this->rowIndex.empty();
    }

    class RowType;
    class RowConstIterator;
    /// Row Sparse Matrix columns constant Iterator to match with constraint matrix manipulation
    class ColConstIterator : std::iterator<std::bidirectional_iterator_tag, Index>
    {
    public:
        friend class RowConstIterator;
        friend class RowType;
    protected:

        ColConstIterator(const Index _rowIt, int _internal, const CompressedRowSparseMatrixConstraint* _matrix)
            : m_rowIt(_rowIt)
            , m_internal(_internal)
            , m_matrix(_matrix)
        {}

    public:

        ColConstIterator(const ColConstIterator& it2)
            : m_rowIt(it2.m_rowIt)
            , m_internal(it2.m_internal)
            , m_matrix(it2.m_matrix)
        {}

        ColConstIterator& operator=(const ColConstIterator& other)
        {
            if (this != &other)
            {
                m_rowIt = other.m_rowIt;
                m_internal = other.m_internal;
                m_matrix = other.m_matrix;
            }
            return *this;
        }

        Index row() const
        {
            return m_matrix->rowIndex[m_rowIt];
        }

        /// @return the constraint value
        const TBloc &val() const
        {
            return m_matrix->colsValue[m_internal];
        }

        /// @return the DOF index the constraint is applied on
        Index index() const
        {
            return m_matrix->colsIndex[m_internal];
        }

        const Index getInternal() const
        {
            return m_internal;
        }

        void operator++() // prefix
        {
            m_internal++;
        }

        void operator++(int) // postfix
        {
            m_internal++;
        }

        void operator--() // prefix
        {
            m_internal--;
        }

        void operator--(int) // postfix
        {
            m_internal--;
        }

        void operator+=(int i)
        {
            m_internal += i;
        }

        void operator-=(int i)
        {
            m_internal -= i;
        }

        bool operator==(const ColConstIterator& it2) const
        {
            return (m_internal == it2.m_internal);
        }

        bool operator!=(const ColConstIterator& it2) const
        {
            return (m_internal != it2.m_internal);
        }

        bool operator<(const ColConstIterator& it2) const
        {
            return m_internal < it2.m_internal;
        }

        bool operator>(const ColConstIterator& it2) const
        {
            return m_internal > it2.m_internal;
        }

    private :

        Index m_rowIt;
        Index m_internal;
        const CompressedRowSparseMatrixConstraint* m_matrix;
    };

    class RowConstIterator : public std::iterator<std::bidirectional_iterator_tag, Index>
    {
    public:

        friend class CompressedRowSparseMatrixConstraint;

    protected:

        RowConstIterator(const CompressedRowSparseMatrixConstraint* _matrix, int _m_internal)
            : m_internal(_m_internal)
            , m_matrix(_matrix)
        {}

    public:

        RowConstIterator(const RowConstIterator& it2)
            : m_internal(it2.m_internal)
            , m_matrix(it2.m_matrix)
        {}

        RowConstIterator()
        {}

        RowConstIterator&  operator=(const RowConstIterator& other)
        {
            if (this != &other)
            {
                m_matrix = other.m_matrix;
                m_internal = other.m_internal;
            }
            return *this;
        }

        Index index() const
        {
            return m_matrix->rowIndex[m_internal];
        }

        Index getInternal() const
        {
            return m_internal;
        }

        ColConstIterator begin() const
        {
            Range r = m_matrix->getRowRange(m_internal);
            return ColConstIterator(m_internal, r.begin(), m_matrix);
        }

        ColConstIterator end() const
        {
            Range r = m_matrix->getRowRange(m_internal);
            return ColConstIterator(m_internal, r.end(), m_matrix);
        }

        RowType row() const
        {
            Range r = m_matrix->getRowRange(m_internal);
            return RowType(ColConstIterator(m_internal, r.begin(), m_matrix),
                           ColConstIterator(m_internal, r.end(), m_matrix));
        }

        bool empty() const
        {
            Range r = m_matrix->getRowRange(m_internal);
            return r.empty();
        }

        void operator++() // prefix
        {
            m_internal++;
        }

        void operator++(int) // postfix
        {
            m_internal++;
        }

        void operator--() // prefix
        {
            m_internal--;
        }

        void operator--(int) // postfix
        {
            m_internal--;
        }

        void operator+=(int i)
        {
            m_internal += i;
        }

        void operator-=(int i)
        {
            m_internal -= i;
        }

        int operator-(const RowConstIterator& it2) const
        {
            return m_internal - it2.m_internal;
        }

        RowConstIterator operator+(int i) const
        {
            RowConstIterator res = *this;
            res += i;
            return res;
        }

        RowConstIterator operator-(int i) const
        {
            RowConstIterator res = *this;
            res -= i;
            return res;
        }

        bool operator==(const RowConstIterator& it2) const
        {
            return m_internal == it2.m_internal;
        }

        bool operator!=(const RowConstIterator& it2) const
        {
            return !(m_internal == it2.m_internal);
        }

        bool operator<(const RowConstIterator& it2) const
        {
            return m_internal < it2.m_internal;
        }

        bool operator>(const RowConstIterator& it2) const
        {
            return m_internal > it2.m_internal;
        }

        template <class VecDeriv, typename Real>
        Real operator*(const VecDeriv& v) const
        {
            return CompressedRowSparseMatrixVecDerivMult(row(), v);
        }

    private:

        Index m_internal;
        const CompressedRowSparseMatrixConstraint* m_matrix;
    };

    /// Get the iterator corresponding to the beginning of the rows of blocks
    RowConstIterator begin() const
    {
        SOFA_IF_CONSTEXPR (Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        return RowConstIterator(this, 0);
    }

    /// Get the iterator corresponding to the end of the rows of blocks
    RowConstIterator end() const
    {
        SOFA_IF_CONSTEXPR (Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        return RowConstIterator(this, this->rowIndex.size());
    }

    /// Get the iterator corresponding to the beginning of the rows of blocks
    RowConstIterator cbegin() const
    {
        SOFA_IF_CONSTEXPR(Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        return RowConstIterator(this, 0);
    }

    /// Get the iterator corresponding to the end of the rows of blocks
    RowConstIterator cend() const
    {
        SOFA_IF_CONSTEXPR(Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        return RowConstIterator(this, this->rowIndex.size());
    }

    class RowWriteAccessor
    {
    public:

        friend class CompressedRowSparseMatrixConstraint;

    protected:

        RowWriteAccessor(CompressedRowSparseMatrixConstraint* _matrix, int _rowIndex)
            : m_rowIndex(_rowIndex)
            , m_matrix(_matrix)
        {}

    public:

        void addCol(Index id, const Bloc& value)
        {
            SOFA_IF_CONSTEXPR (Policy::LogTrace) m_matrix->logCall(FnEnum::addCol, m_rowIndex, id, value);
            *m_matrix->wbloc(m_rowIndex, id, true) += value;
        }

        // TODO: this is wrong in case the returned bloc is within the uncompressed triplets
        void setCol(Index id, const Bloc& value)
        {
            SOFA_IF_CONSTEXPR (Policy::LogTrace) m_matrix->logCall(FnEnum::setCol, m_rowIndex, id, value);
            *m_matrix->wbloc(m_rowIndex, id, true) = value;
        }

        bool operator==(const RowWriteAccessor& it2) const
        {
            return m_rowIndex == it2.m_rowIndex;
        }

        bool operator!=(const RowWriteAccessor& it2) const
        {
            return !(m_rowIndex == it2.m_rowIndex);
        }

    private:
        int m_rowIndex;
        CompressedRowSparseMatrixConstraint* m_matrix;
    };

    typedef RowWriteAccessor RowIterator; /// Definition for MapMapSparseMatrix and CompressedRowSparseMatrixConstraint compatibility

    class RowType : public std::pair<ColConstIterator, ColConstIterator>
    {
        typedef std::pair<ColConstIterator, ColConstIterator> Inherit;
    public:
        RowType(ColConstIterator begin, ColConstIterator end) : Inherit(begin,end) {}
        ColConstIterator begin() const { return this->first; }
        ColConstIterator end() const { return this->second; }
        ColConstIterator cbegin() const { return this->first; }
        ColConstIterator cend() const { return this->second; }
        void setBegin(ColConstIterator i) { this->first = i; }
        void setEnd(ColConstIterator i) { this->second = i; }
        bool empty() const { return begin() == end(); }
        Index size() const { return end().getInternal() - begin().getInternal(); }
        void operator++() { ++this->first; }
        void operator++(int) { ++this->first; }
        ColConstIterator find(Index col) const
        {
            const CompressedRowSparseMatrixConstraint* matrix = this->first.m_matrix;
            Range r(this->first.m_internal, this->second.m_internal);
            Index index = 0;
            if (!matrix->sortedFind(matrix->colsIndex, r, col, index))
            {
                index = r.end(); // not found -> return end
            }
            return ColConstIterator(this->first.m_rowIt, index, matrix);
        }

    };

    /// Get the number of constraint
    size_t size() const
    {
        SOFA_IF_CONSTEXPR(Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        return this->getRowIndex().size();
    }

    /// @return Constant Iterator on specified row
    /// @param lIndex row index
    /// If lIndex row doesn't exist, returns end iterator
    RowConstIterator readLine(Index lIndex) const
    {
        SOFA_IF_CONSTEXPR (Policy::AutoCompress) const_cast<Matrix*>(this)->compress();  /// \warning this violates the const-ness of the method !
        Index rowId = (this->nBlocRow == 0) ? 0 : lIndex * this->rowIndex.size() / this->nBlocRow;
        if (this->sortedFind(this->rowIndex, lIndex, rowId))
        {
            return RowConstIterator(this, rowId);
        }
        else
        {
            return RowConstIterator(this, this->rowIndex.size());
        }
    }

    /// @return Iterator on specified row
    /// @param lIndex row index
    RowWriteAccessor writeLine(Index lIndex)
    {
        return RowWriteAccessor(this, lIndex);
    }

    /// @param lIndex row Index
    /// @param row constraint itself
    /// If lindex already exists, overwrite existing constraint
    void setLine(Index lIndex, RowType row)
    {
        if (readLine(lIndex) != this->end()) this->clearRowBloc(lIndex);

        RowWriteAccessor it(this, lIndex);
        ColConstIterator colIt = row.first;
        ColConstIterator colItEnd = row.second;

        while (colIt != colItEnd)
        {
            it.setCol(colIt.index(), colIt.val());
            ++colIt;
        }
    }

    /// @param lIndex row Index
    /// @param row constraint itself
    /// If lindex doesn't exists, creates the row
    void addLine(Index lIndex, RowType row)
    {
        RowWriteAccessor it(this, lIndex);

        ColConstIterator colIt = row.first;
        ColConstIterator colItEnd = row.second;

        while (colIt != colItEnd)
        {
            it.addCol(colIt.index(), colIt.val());
            ++colIt;
        }
    }

    template< class VecDeriv>
    void multTransposeBaseVector(VecDeriv& res, const sofa::defaulttype::BaseVector* lambda ) const
    {
        typedef typename VecDeriv::value_type Deriv;

        static_assert(std::is_same<Deriv, TBloc>::value, "res must be contain same type as CompressedRowSparseMatrix type");

        for (auto rowIt = begin(), rowItEnd = end(); rowIt != rowItEnd; ++rowIt)
        {
            const SReal f = lambda->element(rowIt.index());
            for (auto colIt = rowIt.begin(), colItEnd = rowIt.end(); colIt != colItEnd; ++colIt)
            {
                res[colIt.index()] += colIt.val() * f;
            }
        }
    }

    /// write to an output stream
    inline friend std::ostream& operator << ( std::ostream& out, const CompressedRowSparseMatrixConstraint<TBloc, Policy>& sc)
    {
        for (RowConstIterator rowIt = sc.begin(); rowIt !=  sc.end(); ++rowIt)
        {
            out << "Constraint ID : ";
            out << rowIt.index();
            for (ColConstIterator colIt = rowIt.begin(); colIt !=  rowIt.end(); ++colIt)
            {
                out << "  dof ID : " << colIt.index() << "  value : " << colIt.val() << "  ";
            }
            out << "\n";
        }

        return out;
    }

    /// read from an input stream
    inline friend std::istream& operator >> ( std::istream& in, CompressedRowSparseMatrixConstraint<TBloc, Policy>& sc)
    {
        sc.clear();

        unsigned int c_id;
        unsigned int c_number;
        unsigned int c_dofIndex;
        TBloc c_value;

        while (!(in.rdstate() & std::istream::eofbit))
        {
            in >> c_id;
            in >> c_number;

            RowIterator c_it = sc.writeLine(c_id);

            for (unsigned int i = 0; i < c_number; i++)
            {
                in >> c_dofIndex;
                in >> c_value;
                c_it.addCol(c_dofIndex, c_value);
            }
        }

        sc.compress();
        return in;
    }

    static const char* Name()
    {
        static std::string name = std::string("CompressedRowSparseMatrixConstraint") + std::string(traits::Name());
        return name.c_str();
    }
};

/// As it is no longer thread-safe to write to different pre-created rows in CompressedRowSparseMatrixConstraint,
/// this helper class allows to create a per-row buffer vector that can be filled in parallel and then copied sequentially
/// to the output matrix.
///
/// Usage (processing values from inMatrix to outMatrix):
///    CompressedRowSparseMatrixConstraintRowsBuffer outRows;
///    outRows.initRows(inMatrix.begin(),inMatrix.end());
///    for_each(inMatrix.begin(), inMatrix.end(), [auto rowIt] { my_compute_fn(rowIt, outRows.writeLine(rowIt))}); // parallelized loop
///    outRows.addToMatrix(outMatrix); // sequential copy to output matrix
template <class TBloc>
class SparseMatrixRowsBuffer
{
    using Index = int;
    using Bloc = TBloc;
    using VecBloc  = typename CRSBlocTraits<Bloc>::VecBloc;
    using VecIndex = typename CRSBlocTraits<Bloc>::VecIndex;
public:
    class RowBuffer
    {
    public:
        void init(Index row)
        {
            m_row = row;
            m_cols.clear();
            m_values.clear();
        }
        void addCol(Index col, const TBloc& val)
        {
            m_cols.push_back(col);
            m_values.push_back(val);
        }
        template <class OutMatrix>
        void addToMatrix(OutMatrix& out)
        {
            if (!m_cols.empty())
            {
                auto rowOut = out.writeLine(m_row);
                for (std::size_t i = 0; i < m_cols.size(); ++i)
                {
                    rowOut.addCol(m_cols[i], m_values[i]);
                }
            }
        }
    protected:
        Index m_row;
        VecIndex m_cols;
        VecBloc m_values;
    };
    template<class RowConstIterator>
    void initRows(const RowConstIterator& rowBegin, const RowConstIterator& rowEnd)
    {
        m_rows.resize(rowEnd-rowBegin);
        for (RowConstIterator rowIt = rowBegin; rowIt != rowEnd; ++rowIt)
        {
            m_rows[std::distance(rowBegin, rowIt)].init(rowIt.index());
        }
    }
    template <class RowConstIterator>
    RowBuffer& writeLine(const RowConstIterator& rowIt)
    {
        return m_rows[rowIt.getInternal()];
    }
    template <class OutMatrix>
    void addToMatrix(OutMatrix& out)
    {
        for (auto& row : m_rows)
        {
            row.addToMatrix(out);
        }
    }
protected:
    helper::vector<RowBuffer> m_rows;
};

#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_DEFAULTTYPE) 

extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec1f>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec2f>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec3f>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec6f>;


extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec1d>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec2d>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec3d>;
extern template class SOFA_DEFAULTTYPE_API CompressedRowSparseMatrixConstraint<Vec6d>;

#endif

} // namespace defaulttype

} // namespace sofa

#endif
