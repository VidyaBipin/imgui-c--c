namespace SimdOpt
{
SIMD_SCOPE_BEGIN(INTRIN_MODE)

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

template<class VecUpdate> struct Op2VecRow
{
    typedef typename VecUpdate::vtype vtype;
    typedef typename vtype::lane_type stype;

    int operator()(const uint8_t* _src1, const uint8_t* _src2, uint8_t* _dst, int width) const
    {
        const stype* src1 = (const stype*)_src1;
        const stype* src2 = (const stype*)_src2;
        stype* dst = (stype*)_dst;
        int i;
        VecUpdate updateOp;

        for (i = 0; i <= width - 4*vtype::nlanes; i += 4*vtype::nlanes)
        {
            const stype* sptr = src1 + i;
            vtype s0 = vx_load(sptr);
            vtype s1 = vx_load(sptr + vtype::nlanes);
            vtype s2 = vx_load(sptr + 2*vtype::nlanes);
            vtype s3 = vx_load(sptr + 3*vtype::nlanes);
            sptr = src2 + i;
            s0 = updateOp(s0, vx_load(sptr));
            s1 = updateOp(s1, vx_load(sptr + vtype::nlanes));
            s2 = updateOp(s2, vx_load(sptr + 2*vtype::nlanes));
            s3 = updateOp(s3, vx_load(sptr + 3*vtype::nlanes));
            v_store(dst + i, s0);
            v_store(dst + i + vtype::nlanes, s1);
            v_store(dst + i + 2*vtype::nlanes, s2);
            v_store(dst + i + 3*vtype::nlanes, s3);
        }
        if (i <= width - 2*vtype::nlanes)
        {
            const stype* sptr = src1 + i;
            vtype s0 = vx_load(sptr);
            vtype s1 = vx_load(sptr + vtype::nlanes);
            sptr = src2 + i;
            s0 = updateOp(s0, vx_load(sptr));
            s1 = updateOp(s1, vx_load(sptr + vtype::nlanes));
            v_store(dst + i, s0);
            v_store(dst + i + vtype::nlanes, s1);
            i += 2*vtype::nlanes;
        }
        if (i <= width - vtype::nlanes)
        {
            vtype s0 = vx_load(src1 + i);
            s0 = updateOp(s0, vx_load(src2 + i));
            v_store(dst + i, s0);
            i += vtype::nlanes;
        }
        if (i <= width - vtype::nlanes/2)
        {
            vtype s0 = vx_load_low(src1 + i);
            s0 = updateOp(s0, vx_load_low(src2 + i));
            v_store_low(dst + i, s0);
            i += vtype::nlanes/2;
        }
        return i;
    }
};

template<class Op, class VecOp> struct MaxSimd : public MatUtils::MatOp2
{
    typedef typename Op::rtype T;

    void operator()(const ImGui::ImMat& _src1, const ImGui::ImMat& _src2, ImGui::ImMat& _dst) override
    {
        int width = _src1.w;
        const int height = _src1.h;
        const int cn = _src1.c;
        const int src1LineSize = _src1.w*_src1.c*_src1.elemsize;
        const int src2LineSize = _src2.w*_src2.c*_src2.elemsize;

        if (_dst.w != _src1.w || _dst.h != _src1.h || _dst.c != _src1.c || _dst.type != _src1.type)
        {
            _dst.release();
            _dst.create_type(_src1.w, _src1.h, _src1.c, _src1.type);
        }
        const int dstLineSize = _dst.w*_dst.c*_dst.elemsize;

        Op op;
        const uint8_t* src1 = (const uint8_t*)_src1.data;
        const uint8_t* src2 = (const uint8_t*)_src2.data;
        uint8_t* dst = (uint8_t*)_dst.data;
        width *= cn;
        int i, j;
        for (j = 0; j < height; j++)
        {
            i = vecOp(src1, src2, dst, width);

            T* D = (T*)dst;
            for (; i <= width - 4; i += 4)
            {
                const T* sptr = (const T*)src1 + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];
                sptr = (const T*)src2 + i;
                s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                D[i  ] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }
            for (; i < width; i++)
            {
                T s0 = *((const T*)src1 + i);
                s0 = op(s0, *((const T*)src2 + i));
                D[i] = s0;
            }

            src1 += src1LineSize;
            src2 += src2LineSize;
            dst += dstLineSize;
        }
    }

    VecOp vecOp;
};

SIMD_SCOPE_END
}