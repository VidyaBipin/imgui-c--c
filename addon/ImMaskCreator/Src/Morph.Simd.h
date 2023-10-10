namespace SimdOpt
{
SIMD_SCOPE_BEGIN(INTRIN_MODE)

template<class VecUpdate> struct MorphRowVec
{
    typedef typename VecUpdate::vtype vtype;
    typedef typename vtype::lane_type stype;

    MorphRowVec(int _ksize, int _anchor) : ksize(_ksize), anchor(_anchor) {}

    int operator()(const uint8_t* src, uint8_t* dst, int width, int cn) const
    {
        int i, k, _ksize = ksize*cn;
        width *= cn;
        VecUpdate updateOp;

        for (i = 0; i <= width - 4*vtype::nlanes; i += 4*vtype::nlanes)
        {
            vtype s0 = vx_load((const stype*)src + i);
            vtype s1 = vx_load((const stype*)src + i + vtype::nlanes);
            vtype s2 = vx_load((const stype*)src + i + 2*vtype::nlanes);
            vtype s3 = vx_load((const stype*)src + i + 3*vtype::nlanes);
            for (k = cn; k < _ksize; k += cn)
            {
                s0 = updateOp(s0, vx_load((const stype*)src + i + k));
                s1 = updateOp(s1, vx_load((const stype*)src + i + k + vtype::nlanes));
                s2 = updateOp(s2, vx_load((const stype*)src + i + k + 2*vtype::nlanes));
                s3 = updateOp(s3, vx_load((const stype*)src + i + k + 3*vtype::nlanes));
            }
            v_store((stype*)dst + i, s0);
            v_store((stype*)dst + i + vtype::nlanes, s1);
            v_store((stype*)dst + i + 2*vtype::nlanes, s2);
            v_store((stype*)dst + i + 3*vtype::nlanes, s3);
        }
        if (i <= width - 2*vtype::nlanes)
        {
            vtype s0 = vx_load((const stype*)src + i);
            vtype s1 = vx_load((const stype*)src + i + vtype::nlanes);
            for( k = cn; k < _ksize; k += cn )
            {
                s0 = updateOp(s0, vx_load((const stype*)src + i + k));
                s1 = updateOp(s1, vx_load((const stype*)src + i + k + vtype::nlanes));
            }
            v_store((stype*)dst + i, s0);
            v_store((stype*)dst + i + vtype::nlanes, s1);
            i += 2*vtype::nlanes;
        }
        if (i <= width - vtype::nlanes)
        {
            vtype s = vx_load((const stype*)src + i);
            for( k = cn; k < _ksize; k += cn )
                s = updateOp(s, vx_load((const stype*)src + i + k));
            v_store((stype*)dst + i, s);
            i += vtype::nlanes;
        }
        if (i <= width - vtype::nlanes/2)
        {
            vtype s = vx_load_low((const stype*)src + i);
            for( k = cn; k < _ksize; k += cn )
                s = updateOp(s, vx_load_low((const stype*)src + i + k));
            v_store_low((stype*)dst + i, s);
            i += vtype::nlanes/2;
        }

        return i - i % cn;
    }

    int ksize, anchor;
};

template<class VecUpdate> struct MorphColumnVec
{
    typedef typename VecUpdate::vtype vtype;
    typedef typename vtype::lane_type stype;

    MorphColumnVec(int _ksize, int _anchor) : ksize(_ksize), anchor(_anchor) {}

    int operator()(const uint8_t** _src, uint8_t* _dst, int dststep, int count, int width) const
    {
        int i = 0, k, _ksize = ksize;
        VecUpdate updateOp;

        auto simdAlignmentWidth = GetSimdPtrAlignmentWidth(ScopeVars::CPU_FEATURE);
        for( i = 0; i < count + ksize - 1; i++ )
            assert(((size_t)_src[i] & (simdAlignmentWidth-1)) == 0);

        const stype** src = (const stype**)_src;
        stype* dst = (stype*)_dst;
        dststep /= sizeof(dst[0]);

        for (; _ksize > 1 && count > 1; count -= 2, dst += dststep*2, src += 2)
        {
            for (i = 0; i <= width - 4*vtype::nlanes; i += 4*vtype::nlanes)
            {
                const stype* sptr = src[1] + i;
                vtype s0 = vx_load_aligned(sptr);
                vtype s1 = vx_load_aligned(sptr + vtype::nlanes);
                vtype s2 = vx_load_aligned(sptr + 2*vtype::nlanes);
                vtype s3 = vx_load_aligned(sptr + 3*vtype::nlanes);

                for (k = 2; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = updateOp(s0, vx_load_aligned(sptr));
                    s1 = updateOp(s1, vx_load_aligned(sptr + vtype::nlanes));
                    s2 = updateOp(s2, vx_load_aligned(sptr + 2*vtype::nlanes));
                    s3 = updateOp(s3, vx_load_aligned(sptr + 3*vtype::nlanes));
                }

                sptr = src[0] + i;
                v_store(dst + i, updateOp(s0, vx_load_aligned(sptr)));
                v_store(dst + i + vtype::nlanes, updateOp(s1, vx_load_aligned(sptr + vtype::nlanes)));
                v_store(dst + i + 2*vtype::nlanes, updateOp(s2, vx_load_aligned(sptr + 2*vtype::nlanes)));
                v_store(dst + i + 3*vtype::nlanes, updateOp(s3, vx_load_aligned(sptr + 3*vtype::nlanes)));

                sptr = src[k] + i;
                v_store(dst + dststep + i, updateOp(s0, vx_load_aligned(sptr)));
                v_store(dst + dststep + i + vtype::nlanes, updateOp(s1, vx_load_aligned(sptr + vtype::nlanes)));
                v_store(dst + dststep + i + 2*vtype::nlanes, updateOp(s2, vx_load_aligned(sptr + 2*vtype::nlanes)));
                v_store(dst + dststep + i + 3*vtype::nlanes, updateOp(s3, vx_load_aligned(sptr + 3*vtype::nlanes)));
            }
            if (i <= width - 2*vtype::nlanes)
            {
                const stype* sptr = src[1] + i;
                vtype s0 = vx_load_aligned(sptr);
                vtype s1 = vx_load_aligned(sptr + vtype::nlanes);

                for (k = 2; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = updateOp(s0, vx_load_aligned(sptr));
                    s1 = updateOp(s1, vx_load_aligned(sptr + vtype::nlanes));
                }

                sptr = src[0] + i;
                v_store(dst + i, updateOp(s0, vx_load_aligned(sptr)));
                v_store(dst + i + vtype::nlanes, updateOp(s1, vx_load_aligned(sptr + vtype::nlanes)));

                sptr = src[k] + i;
                v_store(dst + dststep + i, updateOp(s0, vx_load_aligned(sptr)));
                v_store(dst + dststep + i + vtype::nlanes, updateOp(s1, vx_load_aligned(sptr + vtype::nlanes)));
                i += 2*vtype::nlanes;
            }
            if (i <= width - vtype::nlanes)
            {
                vtype s0 = vx_load_aligned(src[1] + i);

                for (k = 2; k < _ksize; k++)
                    s0 = updateOp(s0, vx_load_aligned(src[k] + i));

                v_store(dst + i, updateOp(s0, vx_load_aligned(src[0] + i)));
                v_store(dst + dststep + i, updateOp(s0, vx_load_aligned(src[k] + i)));
                i += vtype::nlanes;
            }
            if (i <= width - vtype::nlanes/2)
            {
                vtype s0 = vx_load_low(src[1] + i);

                for( k = 2; k < _ksize; k++ )
                    s0 = updateOp(s0, vx_load_low(src[k] + i));

                v_store_low(dst + i, updateOp(s0, vx_load_low(src[0] + i)));
                v_store_low(dst + dststep + i, updateOp(s0, vx_load_low(src[k] + i)));
                i += vtype::nlanes/2;
            }
        }

        for (; count > 0; count--, dst += dststep, src++)
        {
            for (i = 0; i <= width - 4*vtype::nlanes; i += 4*vtype::nlanes)
            {
                const stype* sptr = src[0] + i;
                vtype s0 = vx_load_aligned(sptr);
                vtype s1 = vx_load_aligned(sptr + vtype::nlanes);
                vtype s2 = vx_load_aligned(sptr + 2*vtype::nlanes);
                vtype s3 = vx_load_aligned(sptr + 3*vtype::nlanes);

                for (k = 1; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = updateOp(s0, vx_load_aligned(sptr));
                    s1 = updateOp(s1, vx_load_aligned(sptr + vtype::nlanes));
                    s2 = updateOp(s2, vx_load_aligned(sptr + 2*vtype::nlanes));
                    s3 = updateOp(s3, vx_load_aligned(sptr + 3*vtype::nlanes));
                }
                v_store(dst + i, s0);
                v_store(dst + i + vtype::nlanes, s1);
                v_store(dst + i + 2*vtype::nlanes, s2);
                v_store(dst + i + 3*vtype::nlanes, s3);
            }
            if (i <= width - 2*vtype::nlanes)
            {
                const stype* sptr = src[0] + i;
                vtype s0 = vx_load_aligned(sptr);
                vtype s1 = vx_load_aligned(sptr + vtype::nlanes);

                for (k = 1; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = updateOp(s0, vx_load_aligned(sptr));
                    s1 = updateOp(s1, vx_load_aligned(sptr + vtype::nlanes));
                }
                v_store(dst + i, s0);
                v_store(dst + i + vtype::nlanes, s1);
                i += 2*vtype::nlanes;
            }
            if (i <= width - vtype::nlanes)
            {
                vtype s0 = vx_load_aligned(src[0] + i);

                for( k = 1; k < _ksize; k++ )
                    s0 = updateOp(s0, vx_load_aligned(src[k] + i));
                v_store(dst + i, s0);
                i += vtype::nlanes;
            }
            if (i <= width - vtype::nlanes/2)
            {
                vtype s0 = vx_load_low(src[0] + i);

                for( k = 1; k < _ksize; k++ )
                    s0 = updateOp(s0, vx_load_low(src[k] + i));
                v_store_low(dst + i, s0);
                i += vtype::nlanes/2;
            }
        }

        return i;
    }

    int ksize, anchor;
};

template<class VecUpdate> struct MorphVec
{
    typedef typename VecUpdate::vtype vtype;
    typedef typename vtype::lane_type stype;

    int operator()(uint8_t** _src, int nz, uint8_t* _dst, int width) const
    {
        const stype** src = (const stype**)_src;
        stype* dst = (stype*)_dst;
        int i, k;
        VecUpdate updateOp;

        for (i = 0; i <= width - 4*vtype::nlanes; i += 4*vtype::nlanes)
        {
            const stype* sptr = src[0] + i;
            vtype s0 = vx_load(sptr);
            vtype s1 = vx_load(sptr + vtype::nlanes);
            vtype s2 = vx_load(sptr + 2*vtype::nlanes);
            vtype s3 = vx_load(sptr + 3*vtype::nlanes);
            for (k = 1; k < nz; k++)
            {
                sptr = src[k] + i;
                s0 = updateOp(s0, vx_load(sptr));
                s1 = updateOp(s1, vx_load(sptr + vtype::nlanes));
                s2 = updateOp(s2, vx_load(sptr + 2*vtype::nlanes));
                s3 = updateOp(s3, vx_load(sptr + 3*vtype::nlanes));
            }
            v_store(dst + i, s0);
            v_store(dst + i + vtype::nlanes, s1);
            v_store(dst + i + 2*vtype::nlanes, s2);
            v_store(dst + i + 3*vtype::nlanes, s3);
        }
        if (i <= width - 2*vtype::nlanes)
        {
            const stype* sptr = src[0] + i;
            vtype s0 = vx_load(sptr);
            vtype s1 = vx_load(sptr + vtype::nlanes);
            for (k = 1; k < nz; k++)
            {
                sptr = src[k] + i;
                s0 = updateOp(s0, vx_load(sptr));
                s1 = updateOp(s1, vx_load(sptr + vtype::nlanes));
            }
            v_store(dst + i, s0);
            v_store(dst + i + vtype::nlanes, s1);
            i += 2*vtype::nlanes;
        }
        if (i <= width - vtype::nlanes)
        {
            vtype s0 = vx_load(src[0] + i);
            for( k = 1; k < nz; k++ )
                s0 = updateOp(s0, vx_load(src[k] + i));
            v_store(dst + i, s0);
            i += vtype::nlanes;
        }
        if (i <= width - vtype::nlanes/2)
        {
            vtype s0 = vx_load_low(src[0] + i);
            for (k = 1; k < nz; k++)
                s0 = updateOp(s0, vx_load_low(src[k] + i));
            v_store_low(dst + i, s0);
            i += vtype::nlanes/2;
        }
        return i;
    }
};

template <typename T> struct VMin
{
    typedef T vtype;
    vtype operator()(const vtype& a, const vtype& b) const { return v_min(a,b); }
};
template <typename T> struct VMax
{
    typedef T vtype;
    vtype operator()(const vtype& a, const vtype& b) const { return v_max(a, b); }
};

template<class Op, class VecOp> struct MorphRowFilter : public MatUtils::RowFilter
{
    typedef typename Op::rtype T;

    MorphRowFilter(int _ksize, int _anchor) : vecOp(_ksize, _anchor)
    {
        ksize = _ksize;
        anchor = _anchor;
    }

    void operator()(const uint8_t* src, uint8_t* dst, int width, int cn) override
    {
        int i, j, k, _ksize = ksize*cn;
        const T* S = (const T*)src;
        Op op;
        T* D = (T*)dst;

        if (_ksize == cn)
        {
            for (i = 0; i < width*cn; i++)
                D[i] = S[i];
            return;
        }

        int i0 = vecOp(src, dst, width, cn);
        width *= cn;

        for (k = 0; k < cn; k++, S++, D++)
        {
            for (i = i0; i <= width - cn*2; i += cn*2)
            {
                const T* s = S + i;
                T m = s[cn];
                for (j = cn*2; j < _ksize; j += cn)
                    m = op(m, s[j]);
                D[i] = op(m, s[0]);
                D[i+cn] = op(m, s[j]);
            }

            for (; i < width; i += cn)
            {
                const T* s = S + i;
                T m = s[0];
                for (j = cn; j < _ksize; j += cn)
                    m = op(m, s[j]);
                D[i] = m;
            }
        }
    }

    VecOp vecOp;
};

template<class Op, class VecOp> struct MorphColumnFilter : public MatUtils::ColumnFilter
{
    typedef typename Op::rtype T;

    MorphColumnFilter(int _ksize, int _anchor) : vecOp(_ksize, _anchor)
    {
        ksize = _ksize;
        anchor = _anchor;
    }

    void operator()(const uint8_t** _src, uint8_t* dst, int dststep, int count, int width) override
    {
        int i, k, _ksize = ksize;
        const T** src = (const T**)_src;
        T* D = (T*)dst;
        Op op;

        int i0 = vecOp(_src, dst, dststep, count, width);
        dststep /= sizeof(D[0]);

        for (; _ksize > 1 && count > 1; count -= 2, D += dststep*2, src += 2)
        {
            i = i0;
            for (; i <= width - 4; i += 4)
            {
                const T* sptr = src[1] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for (k = 2; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                sptr = src[0] + i;
                D[i] = op(s0, sptr[0]);
                D[i+1] = op(s1, sptr[1]);
                D[i+2] = op(s2, sptr[2]);
                D[i+3] = op(s3, sptr[3]);

                sptr = src[k] + i;
                D[i+dststep] = op(s0, sptr[0]);
                D[i+dststep+1] = op(s1, sptr[1]);
                D[i+dststep+2] = op(s2, sptr[2]);
                D[i+dststep+3] = op(s3, sptr[3]);
            }
            for (; i < width; i++)
            {
                T s0 = src[1][i];

                for (k = 2; k < _ksize; k++)
                    s0 = op(s0, src[k][i]);

                D[i] = op(s0, src[0][i]);
                D[i+dststep] = op(s0, src[k][i]);
            }
        }

        for (; count > 0; count--, D += dststep, src++)
        {
            i = i0;
            for (; i <= width - 4; i += 4)
            {
                const T* sptr = src[0] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for (k = 1; k < _ksize; k++)
                {
                    sptr = src[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }
            for (; i < width; i++)
            {
                T s0 = src[0][i];
                for (k = 1; k < _ksize; k++)
                    s0 = op(s0, src[k][i]);
                D[i] = s0;
            }
        }
    }

    VecOp vecOp;
};

template<class Op, class VecOp> struct MorphFilter : public MatUtils::MatFilter
{
    typedef typename Op::rtype T;

    MorphFilter(const ImGui::ImMat& _kernel, const MatUtils::Point2i& _anchor)
    {
        anchor = _anchor;
        ksize = MatUtils::Size2i(_kernel.w, _kernel.h);

        std::vector<uint8_t> coeffs;
        // kernel elements, just their locations
        Preprocess2DKernel(_kernel, coords, coeffs);
        ptrs.resize(coords.size());
    }

    void operator()(const uint8_t** src, uint8_t* dst, int dststep, int count, int width, int cn) override
    {
        const MatUtils::Point2i* pt = &coords[0];
        const T** kp = (const T**)&ptrs[0];
        int i, k, nz = (int)coords.size();
        Op op;

        width *= cn;
        for (; count > 0; count--, dst += dststep, src++)
        {
            T* D = (T*)dst;

            for (k = 0; k < nz; k++)
                kp[k] = (const T*)src[pt[k].y] + pt[k].x*cn;

            i = vecOp(&ptrs[0], nz, dst, width);
            for (; i <= width - 4; i += 4)
            {
                const T* sptr = kp[0] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for (k = 1; k < nz; k++)
                {
                    sptr = kp[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }
            for (; i < width; i++)
            {
                T s0 = kp[0][i];
                for (k = 1; k < nz; k++)
                    s0 = op(s0, kp[k][i]);
                D[i] = s0;
            }
        }
    }

    std::vector<MatUtils::Point2i> coords;
    std::vector<uint8_t*> ptrs;
    VecOp vecOp;
};

SIMD_SCOPE_END
}