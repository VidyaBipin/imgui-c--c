#include <immat.h>

namespace ImGui
{
void ImMat::get_pixel(int x, int y, ImPixel& color) const
{
    assert(dims == 3);
    assert(x >= 0 && x < w);
    assert(y >= 0 && y < h);
    switch (type)
    {
        case IM_DT_INT8:
            if (c > 0) color.r = (float)at<uint8_t>(x, y, 0) / UINT8_MAX;
            if (c > 1) color.g = (float)at<uint8_t>(x, y, 1) / UINT8_MAX;
            if (c > 2) color.b = (float)at<uint8_t>(x, y, 2) / UINT8_MAX;
            if (c > 3) color.a = (float)at<uint8_t>(x, y, 3) / UINT8_MAX;
        break;
        case IM_DT_INT16:
            if (c > 0) color.r = (float)at<uint16_t>(x, y, 0) / UINT16_MAX;
            if (c > 1) color.g = (float)at<uint16_t>(x, y, 1) / UINT16_MAX;
            if (c > 2) color.b = (float)at<uint16_t>(x, y, 2) / UINT16_MAX;
            if (c > 3) color.a = (float)at<uint16_t>(x, y, 3) / UINT16_MAX;
        break;
        case IM_DT_INT32:
            if (c > 0) color.r = (float)at<uint32_t>(x, y, 0) / (float)UINT32_MAX;
            if (c > 1) color.g = (float)at<uint32_t>(x, y, 1) / (float)UINT32_MAX;
            if (c > 2) color.b = (float)at<uint32_t>(x, y, 2) / (float)UINT32_MAX;
            if (c > 3) color.a = (float)at<uint32_t>(x, y, 3) / (float)UINT32_MAX;
        break;
        case IM_DT_INT64:
            if (c > 0) color.r = (float)at<uint64_t>(x, y, 0) / (float)UINT64_MAX;
            if (c > 1) color.g = (float)at<uint64_t>(x, y, 1) / (float)UINT64_MAX;
            if (c > 2) color.b = (float)at<uint64_t>(x, y, 2) / (float)UINT64_MAX;
            if (c > 3) color.a = (float)at<uint64_t>(x, y, 3) / (float)UINT64_MAX;
        break;
        case IM_DT_FLOAT16:
            // TODO::Dicky add FLOAT16 get pixel from ImPixel
        break;
        case IM_DT_FLOAT32:
            if (c > 0) color.r = at<float>(x, y, 0);
            if (c > 1) color.g = at<float>(x, y, 1);
            if (c > 2) color.b = at<float>(x, y, 2);
            if (c > 3) color.a = at<float>(x, y, 3);
        break;
        case IM_DT_FLOAT64:
            if (c > 0) color.r = (float)at<double>(x, y, 0);
            if (c > 1) color.g = (float)at<double>(x, y, 1);
            if (c > 2) color.b = (float)at<double>(x, y, 2);
            if (c > 3) color.a = (float)at<double>(x, y, 3);
        break;
        default: break;
    }
}

void ImMat::get_pixel(ImPoint p, ImPixel& color) const
{
    get_pixel((int)p.x, (int)p.y, color);
}

ImPixel ImMat::get_pixel(int x, int y) const
{
    ImPixel color;
    get_pixel(x, y, color);
    return color;
}

ImPixel ImMat::get_pixel(ImPoint p) const
{
    return get_pixel((int)p.x, (int)p.y);
}

void ImMat::draw_dot(int x, int y, ImPixel color)
{
    assert(dims == 3);
    //assert(x >= 0 && x < w);
    //assert(y >= 0 && y < h);
    if (x < 0 || x >= w || y < 0 || y >= h)
        return;
    switch (type)
    {
        case IM_DT_INT8:
            if (c > 0) at<uint8_t>(x, y, 0) = color.r * UINT8_MAX;
            if (c > 1) at<uint8_t>(x, y, 1) = color.g * UINT8_MAX;
            if (c > 2) at<uint8_t>(x, y, 2) = color.b * UINT8_MAX;
            if (c > 3) at<uint8_t>(x, y, 3) = color.a * UINT8_MAX;
        break;
        case IM_DT_INT16:
            if (c > 0) at<uint16_t>(x, y, 0) = color.r * UINT16_MAX;
            if (c > 1) at<uint16_t>(x, y, 1) = color.g * UINT16_MAX;
            if (c > 2) at<uint16_t>(x, y, 2) = color.b * UINT16_MAX;
            if (c > 3) at<uint16_t>(x, y, 3) = color.a * UINT16_MAX;
        break;
        case IM_DT_INT32:
            if (c > 0) at<uint32_t>(x, y, 0) = color.r * (float)UINT32_MAX;
            if (c > 1) at<uint32_t>(x, y, 1) = color.g * (float)UINT32_MAX;
            if (c > 2) at<uint32_t>(x, y, 2) = color.b * (float)UINT32_MAX;
            if (c > 3) at<uint32_t>(x, y, 3) = color.a * (float)UINT32_MAX;
        break;
        case IM_DT_INT64:
            if (c > 0) at<uint64_t>(x, y, 0) = color.r * (float)UINT64_MAX;
            if (c > 1) at<uint64_t>(x, y, 1) = color.g * (float)UINT64_MAX;
            if (c > 2) at<uint64_t>(x, y, 2) = color.b * (float)UINT64_MAX;
            if (c > 3) at<uint64_t>(x, y, 3) = color.a * (float)UINT64_MAX;
        break;
        case IM_DT_FLOAT16:
            // TODO::Dicky add FLOAT16 draw dot
        break;
        case IM_DT_FLOAT32:
            if (c > 0) at<float>(x, y, 0) = color.r;
            if (c > 1) at<float>(x, y, 1) = color.g;
            if (c > 2) at<float>(x, y, 2) = color.b;
            if (c > 3) at<float>(x, y, 3) = color.a;
        break;
        case IM_DT_FLOAT64:
            if (c > 0) at<double>(x, y, 0) = (double)color.r;
            if (c > 1) at<double>(x, y, 1) = (double)color.g;
            if (c > 2) at<double>(x, y, 2) = (double)color.b;
            if (c > 3) at<double>(x, y, 3) = (double)color.a;
        break;
        default: break;
    }
}

void ImMat::draw_dot(ImPoint p, ImPixel color)
{
    draw_dot((int)p.x, (int)p.y, color);
}

void ImMat::alphablend(int x, int y, float alpha, ImPixel color)
{
    switch (type)
    {
        case IM_DT_INT8:
        {
            float alpha_org = c > 2 ? at<uint8_t>(x, y, 3) / (float)UINT8_MAX : 1;
            if (c > 0) at<uint8_t>(x, y, 0) = at<uint8_t>(x, y, 0) * (1 - alpha) + color.r * alpha * UINT8_MAX;
            if (c > 1) at<uint8_t>(x, y, 1) = at<uint8_t>(x, y, 1) * (1 - alpha) + color.g * alpha * UINT8_MAX;
            if (c > 2) at<uint8_t>(x, y, 2) = at<uint8_t>(x, y, 2) * (1 - alpha) + color.b * alpha * UINT8_MAX;
            if (c > 3) at<uint8_t>(x, y, 3) = (uint8_t)(CLAMP(color.a * alpha + alpha_org, 0.f, 1.f) * UINT8_MAX);
        }
        break;
        case IM_DT_INT16:
        {
            float alpha_org = c > 2 ? at<uint16_t>(x, y, 3) / (float)UINT16_MAX : 1;
            if (c > 0) at<uint16_t>(x, y, 0) = at<uint16_t>(x, y, 0) * (1 - alpha) + color.r * alpha * UINT16_MAX;
            if (c > 1) at<uint16_t>(x, y, 1) = at<uint16_t>(x, y, 1) * (1 - alpha) + color.g * alpha * UINT16_MAX;
            if (c > 2) at<uint16_t>(x, y, 2) = at<uint16_t>(x, y, 2) * (1 - alpha) + color.b * alpha * UINT16_MAX;
            if (c > 3) at<uint16_t>(x, y, 3) = (uint16_t)(CLAMP(color.a * alpha + alpha_org, 0.f, 1.f) * UINT16_MAX);
        }
        break;
        case IM_DT_INT32:
        {
            float alpha_org = c > 2 ? at<uint16_t>(x, y, 3) / (float)UINT32_MAX : 1;
            if (c > 0) at<uint32_t>(x, y, 0) = at<uint32_t>(x, y, 0) * (1 - alpha) + color.r * alpha * (float)UINT32_MAX;
            if (c > 1) at<uint32_t>(x, y, 1) = at<uint32_t>(x, y, 1) * (1 - alpha) + color.g * alpha * (float)UINT32_MAX;
            if (c > 2) at<uint32_t>(x, y, 2) = at<uint32_t>(x, y, 2) * (1 - alpha) + color.b * alpha * (float)UINT32_MAX;
            if (c > 3) at<uint32_t>(x, y, 3) = (uint32_t)(CLAMP(color.a * alpha + alpha_org, 0.f, 1.f) * (float)UINT32_MAX);
        }
        break;
        case IM_DT_INT64:
        {
            float alpha_org = c > 2 ? at<uint64_t>(x, y, 3) / (float)UINT64_MAX : 1;
            if (c > 0) at<uint64_t>(x, y, 0) = at<uint64_t>(x, y, 0) * (1 - alpha) + color.r * alpha * (float)UINT64_MAX;
            if (c > 1) at<uint64_t>(x, y, 1) = at<uint64_t>(x, y, 1) * (1 - alpha) + color.g * alpha * (float)UINT64_MAX;
            if (c > 2) at<uint64_t>(x, y, 2) = at<uint64_t>(x, y, 2) * (1 - alpha) + color.b * alpha * (float)UINT64_MAX;
            if (c > 3) at<uint64_t>(x, y, 3) = (uint64_t)(CLAMP(color.a * alpha + alpha_org, 0.f, 1.f) * (float)UINT64_MAX);
        }
        break;
        case IM_DT_FLOAT16:
            // TODO::Dicky add FLOAT16 alphablend
        break;
        case IM_DT_FLOAT32:
        {
            float alpha_org = c > 2 ? at<float>(x, y, 3) : 1;
            if (c > 0) at<float>(x, y, 0) = at<float>(x, y, 0) * (1 - alpha) + color.r * alpha;
            if (c > 1) at<float>(x, y, 1) = at<float>(x, y, 1) * (1 - alpha) + color.g * alpha;
            if (c > 2) at<float>(x, y, 2) = at<float>(x, y, 2) * (1 - alpha) + color.b * alpha;
            if (c > 3) at<float>(x, y, 3) = CLAMP(color.a * alpha + alpha_org, 0.f, 1.f);
        }
        break;
        case IM_DT_FLOAT64:
        {
            double alpha_org = c > 2 ? at<double>(x, y, 3) : 1;
            if (c > 0) at<double>(x, y, 0) = at<double>(x, y, 0) * (1 - alpha) + color.r * alpha;
            if (c > 1) at<double>(x, y, 1) = at<double>(x, y, 1) * (1 - alpha) + color.g * alpha;
            if (c > 2) at<double>(x, y, 2) = at<double>(x, y, 2) * (1 - alpha) + color.b * alpha;
            if (c > 3) at<double>(x, y, 3) = (double)(CLAMP(color.a * alpha + (float)alpha_org, 0.f, 1.f));
        }
        break;
        default: break;
    }
}

void ImMat::alphablend(int x, int y, ImPixel color)
{
    switch (type)
    {
        case IM_DT_INT8:
        {
            //float alpha_org = c > 2 ? at<uint8_t>(x, y, 3) / (float)UINT8_MAX : 1;
            float alpha = color.a * UINT8_MAX;
            if (c > 0) at<uint8_t>(x, y, 0) = at<uint8_t>(x, y, 0) * (1 - color.a) + color.r * alpha;
            if (c > 1) at<uint8_t>(x, y, 1) = at<uint8_t>(x, y, 1) * (1 - color.a) + color.g * alpha;
            if (c > 2) at<uint8_t>(x, y, 2) = at<uint8_t>(x, y, 2) * (1 - color.a) + color.b * alpha;
            if (c > 3) at<uint8_t>(x, y, 3) = UINT8_MAX; //(uint8_t)(CLAMP(color.a + alpha_org, 0.f, 1.f) * UINT8_MAX);
        }
        break;
        case IM_DT_INT16:
        {
            //float alpha_org = c > 2 ? at<uint16_t>(x, y, 3) / (float)UINT16_MAX : 1;
            float alpha = color.a * UINT16_MAX;
            if (c > 0) at<uint16_t>(x, y, 0) = at<uint16_t>(x, y, 0) * (1 - color.a) + color.r * alpha;
            if (c > 1) at<uint16_t>(x, y, 1) = at<uint16_t>(x, y, 1) * (1 - color.a) + color.g * alpha;
            if (c > 2) at<uint16_t>(x, y, 2) = at<uint16_t>(x, y, 2) * (1 - color.a) + color.b * alpha;
            if (c > 3) at<uint16_t>(x, y, 3) = UINT16_MAX;//(uint16_t)(CLAMP(color.a + alpha_org, 0.f, 1.f) * UINT16_MAX);
        }
        break;
        case IM_DT_INT32:
        {
            //float alpha_org = c > 2 ? at<uint16_t>(x, y, 3) / (float)UINT32_MAX : 1;
            float alpha = color.a * (float)UINT32_MAX;
            if (c > 0) at<uint32_t>(x, y, 0) = at<uint32_t>(x, y, 0) * (1 - color.a) + color.r * alpha;
            if (c > 1) at<uint32_t>(x, y, 1) = at<uint32_t>(x, y, 1) * (1 - color.a) + color.g * alpha;
            if (c > 2) at<uint32_t>(x, y, 2) = at<uint32_t>(x, y, 2) * (1 - color.a) + color.b * alpha;
            if (c > 3) at<uint32_t>(x, y, 3) = UINT32_MAX;//(uint32_t)(CLAMP(color.a + alpha_org, 0.f, 1.f) * (float)UINT32_MAX);
        }
        break;
        case IM_DT_INT64:
        {
            //float alpha_org = c > 2 ? at<uint64_t>(x, y, 3) / (float)UINT64_MAX : 1;
            float alpha = color.a * (float)UINT64_MAX;
            if (c > 0) at<uint64_t>(x, y, 0) = at<uint64_t>(x, y, 0) * (1 - color.a) + color.r * alpha;
            if (c > 1) at<uint64_t>(x, y, 1) = at<uint64_t>(x, y, 1) * (1 - color.a) + color.g * alpha;
            if (c > 2) at<uint64_t>(x, y, 2) = at<uint64_t>(x, y, 2) * (1 - color.a) + color.b * alpha;
            if (c > 3) at<uint64_t>(x, y, 3) = UINT64_MAX;//(uint64_t)(CLAMP(color.a + alpha_org, 0.f, 1.f) * (float)UINT64_MAX);
        }
        break;
        case IM_DT_FLOAT16:
            // TODO::Dicky add FLOAT16 alphablend
        break;
        case IM_DT_FLOAT32:
        {
            //float alpha_org = c > 2 ? at<float>(x, y, 3) : 1;
            if (c > 0) at<float>(x, y, 0) = at<float>(x, y, 0) * (1 - color.a) + color.r * color.a;
            if (c > 1) at<float>(x, y, 1) = at<float>(x, y, 1) * (1 - color.a) + color.g * color.a;
            if (c > 2) at<float>(x, y, 2) = at<float>(x, y, 2) * (1 - color.a) + color.b * color.a;
            if (c > 3) at<float>(x, y, 3) = 1.f;//CLAMP(color.a + alpha_org, 0.f, 1.f);
        }
        break;
        case IM_DT_FLOAT64:
        {
            //double alpha_org = c > 2 ? at<double>(x, y, 3) : 1;
            if (c > 0) at<double>(x, y, 0) = at<double>(x, y, 0) * (1 - color.a) + color.r * color.a;
            if (c > 1) at<double>(x, y, 1) = at<double>(x, y, 1) * (1 - color.a) + color.g * color.a;
            if (c > 2) at<double>(x, y, 2) = at<double>(x, y, 2) * (1 - color.a) + color.b * color.a;
            if (c > 3) at<double>(x, y, 3) = 1.0; //(double)(CLAMP(color.a + alpha_org, 0.0, 1.0));
        }
        break;
        default: break;
    }
}

void ImMat::draw_line(float x1, float y1, float x2, float y2, float t, ImPixel color)
{
    assert(dims == 3);

    int _x0 = CLAMP((int)floorf(fminf(x1, x2) - t), 0, w - 1);
    int _x1 = CLAMP((int) ceilf(fmaxf(x1, x2) + t), 0, w - 1);
    int _y0 = CLAMP((int)floorf(fminf(y1, y2) - t), 0, h - 1);
    int _y1 = CLAMP((int) ceilf(fmaxf(y1, y2) + t), 0, h - 1);
    for (int y = _y0; y <= _y1; y++)
    {
        for (int x = _x0; x <= _x1; x++)
        {
            // capsuleSDF
            float pax = (float)x - x1, pay = (float)y - y1, bax = x2 - x1, bay = y2 - y1;
            float _h = CLAMP((pax * bax + pay * bay) / (bax * bax + bay * bay), 0.0f, 1.0f);
            float dx = pax - bax * _h, dy = pay - bay * _h;
            float sdf = sqrtf(dx * dx + dy * dy) - t;
            float alpha = CLAMP(0.5f - sdf, 0.f, 1.f);
            alphablend(x, y, alpha, color);
        }
    }
}

void ImMat::draw_line(ImPoint p1, ImPoint p2, float t, ImPixel color)
{
    draw_line(p1.x, p1.y, p2.x, p2.y, t, color);
}

void ImMat::draw_line(float x1, float y1, float x2, float y2, ImPixel color)
{
    // Bresenham
    x1 = CLAMP(x1, 0.f, w - 1.f);
    x2 = CLAMP(x2, 0.f, w - 1.f);
    y1 = CLAMP(y1, 0.f, h - 1.f);
    y2 = CLAMP(y2, 0.f, h - 1.f);
    int x = x1;
	int y = y1;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int s1 = x2 > x1 ? 1 : -1;
	int s2 = y2 > y1 ? 1 : -1;

	char interchange = 0;
	if (dy > dx)
	{
		int temp = dx;
		dx = dy;
		dy = temp;
		interchange = 1;
	}

	int p = 2 * dy - dx;
	for(int i = 0; i < dx; i++)
	{
        alphablend(x, y, color);
		if (p >= 0)
		{
			if (!interchange)
				y += s2;
			else
				x += s1;
			p -= 2 * dx;
		}
		if (!interchange)
			x += s1;
		else
			y += s2;
		p += 2 * dy;
	}
}

void ImMat::draw_line(ImPoint p1, ImPoint p2, ImPixel color)
{
    draw_line(p1.x, p1.y, p2.x, p2.y, color);
}

void ImMat::draw_rectangle(float x1, float y1, float x2, float y2, ImPixel color)
{
    draw_line(x1, y1, x1, y2, color);
    draw_line(x1, y2, x2, y2, color);
    draw_line(x2, y2, x2, y1, color);
    draw_line(x2, y1, x1, y1, color);
}

void ImMat::draw_rectangle(ImPoint p1, ImPoint p2, ImPixel color)
{
    draw_rectangle(p1.x, p1.y, p2.x, p2.y, color);
}

void ImMat::draw_circle(float x1, float y1, float r, ImPixel color)
{
    // Bresenham circle
    float x = 0, y = r;
	float p = 3 - (2 * r);
	while (x <= y)
	{
        draw_dot(x1 + x, y1 + y, color);
        draw_dot(x1 - x, y1 + y, color);
        draw_dot(x1 + x, y1 - y, color);
        draw_dot(x1 - x, y1 - y, color);
        draw_dot(x1 + y, y1 + x, color);
		draw_dot(x1 + y, y1 - x, color);
		draw_dot(x1 - y, y1 + x, color);
		draw_dot(x1 - y, y1 - x, color);
		x = x + 1;
		if (p < 0)
			p = p + 4 * x + 6;
		else
		{
			p = p + 4 * (x - y) + 10;
			y = y - 1;
        }
    }
}

void ImMat::draw_circle(ImPoint p, float r, ImPixel color)
{
    draw_circle(p.x, p.y, r, color);
}

void ImMat::draw_circle(float x1, float y1, float r, float t, ImPixel color)
{
    float perimeter = 2 * M_PI * r;
    int num_segments = perimeter / 8 / t;
    const float a_max = (M_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    const float a_min = 0;
    float x, y, x0, y0, _x, _y;
    num_segments--;
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        x = x1 + cos(a) * r;
        y = y1 + sin(a) * r;
        if (i == 0)
        {
            x0 = _x = x;
            y0 = _y = y;
        }
        else
        {
            draw_line(_x, _y, x, y, t, color);
            _x = x;
            _y = y;
        }
    }
    draw_line(x, y, x0, y0, t, color);
}

void ImMat::draw_circle(ImPoint p, float r, float t, ImPixel color)
{
    draw_circle(p.x, p.y, r, t, color);
}


ImMat ImMat::lowpass(float lambda)
{
    assert(device == IM_DD_CPU);
    assert(dims == 2);
    assert(w > 0 && h > 0);
    ImMat m = clone();
    if (lambda < FLT_EPSILON)
        return m;
    float B = 1 + 2 / (lambda * lambda);
    float c = B - sqrt(B * B - 1);
    float d = 1 - c;
    #pragma omp parallel for num_threads(OMP_THREADS)
    for (int y = 0; y < h; y++)
    {
        /* apply low-pass filter to row y */
        /* left-to-right */
        float f = 0, g = 0;
        for (int x = 0; x < w; x++)
        {
            f = f * c + (float)m.at<uint8_t>(x, y) * d;
            g = g * c + f * d;
            m.at<uint8_t>(x, y) = (uint8_t)g;
        }
        /* right-to-left */
        for (int x = w - 1; x >= 0; x--)
        {
            f = f * c + (float)m.at<uint8_t>(x, y) * d;
            g = g * c + f * d;
            m.at<uint8_t>(x, y) = (uint8_t)g;
        }

        /* left-to-right mop-up */
        for (int x = 0; x < w; x++)
        {
            f = f * c;
            g = g * c + f * d;
            if (f + g < 1 / 255.0) break;
            m.at<uint8_t>(x, y) = (uint8_t)((float)m.at<uint8_t>(x, y) + g);
        }
    }
    #pragma omp parallel for num_threads(OMP_THREADS)
    for (int x = 0; x < w; x++)
    {
        /* apply low-pass filter to column x */
        /* bottom-to-top */
        float f = 0, g = 0;
        for (int y = 0; y < h; y++)
        {
            f = f * c + (float)m.at<uint8_t>(x, y) * d;
            g = g * c + f * d;
            m.at<uint8_t>(x, y) = (uint8_t)g;
        }

        /* top-to-bottom */
        for (int y = h - 1; y >= 0; y--)
        {
            f = f * c + (float)m.at<uint8_t>(x, y) * d;
            g = g * c + f * d;
            m.at<uint8_t>(x, y) = (uint8_t)g;
        }

        /* bottom-to-top mop-up */
        for (int y = 0; y < h; y++)
        {
            f = f * c;
            g = g * c + f * d;
            if (f + g < 1 / 255.0) break;
            m.at<uint8_t>(x, y) = (uint8_t)((float)m.at<uint8_t>(x, y) + g);
        }
    }
    return m;
}

ImMat ImMat::highpass(float lambda)
{
    assert(device == IM_DD_CPU);
    assert(dims == 2);
    /* apply lowpass filter to the copy */
    ImMat m = lowpass(lambda);
    if (lambda < FLT_EPSILON)
        return m;
    /* subtract copy from original */
    #pragma omp parallel for num_threads(OMP_THREADS)
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float f = (float)at<uint8_t>(x, y);
            f -= (float)m.at<uint8_t>(x, y);
            f += 128; /* normalize! */
            m.at<uint8_t>(x, y) = (uint8_t)f;
        }
    }
    return m;
}

ImMat ImMat::threshold(float thres)
{
    assert(device == IM_DD_CPU);
    assert(dims == 2);
    ImMat m = clone();
    float _thres = thres * 255;
    #pragma omp parallel for num_threads(OMP_THREADS)
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float p = (float)at<uint8_t>(x, y);
            m.at<uint8_t>(x, y) = p < _thres ? 0 : 255;
        }
    }
    return ImMat();
}

ImMat ImMat::resize(float factor)
{
    assert(dims == 2);
    assert(w > 0 && h > 0);
    assert(factor > 0);
    ImMat m((int)(w * factor), (int)(h * factor), (size_t)1);
    /* interpolate */
    int p00, p01, p10, p11;
    double xx, yy, av;
    double p0, p1;
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            p00 = at<int8_t>(i, j);
            p01 = at<int8_t>(i, j + 1);
            p10 = at<int8_t>(i + 1, j);
            p11 = at<int8_t>(i + 1, j + 1);

            /* the general case */
            #pragma omp parallel for num_threads(OMP_THREADS)
            for (int x = 0; x < factor; x++)
            {
                xx = x / (double)factor;
                p0 = p00 * (1 - xx) + p10 * xx;
                p1 = p01 * (1 - xx) + p11 * xx;
                for (int y = 0; y < factor; y++)
                {
                    yy = y / (double)factor;
                    av = p0 * (1 - yy) + p1 * yy;
                    m.at<int8_t>(i * factor + x, j * factor + y) = av;
                }
            }
        }
    }
    return m;
}

void ImMat::copy_to(ImMat & mat, ImPoint offset, float alpha)
{
    // assert mat same as this
    assert(!empty() && !mat.empty());
    assert(offset.x + w >= 0 && offset.y + h >= 0 &&
            offset.x < mat.w && offset.y < mat.h);
    ImSize size;
    ImPoint offset_src;
    ImPoint offset_dst;
    if (offset.x < 0)
    {
        offset_src.x = -offset.x;
        offset_dst.x = 0;
        size.w = std::min(w + (int)offset.x, mat.w);
    }
    else
    {
        offset_src.x = 0;
        offset_dst.x = offset.x;
        size.w = std::min(mat.w - (int)offset.x, w);
    }
    if (offset.y < 0)
    {
        offset_src.y = -offset.y;
        offset_dst.y = 0;
        size.h = std::min(h + (int)offset.y, mat.h);
    }
    else
    {
        offset_src.y = 0;
        offset_dst.y = offset.y;
        size.h = std::min(mat.h - (int)offset.y, h);
    }
    for (int i = 0; i < size.h; i++)
    {
        for (int j = 0; j < size.w; j++)
        {
            auto src_color = get_pixel(offset_src.x + j, offset_src.y + i);
            mat.alphablend(offset_dst.x + j, offset_dst.y + i, alpha, src_color);
        }
    }
}

void ImMat::clean(ImPixel color)
{
    assert(dims == 3);
    assert(c > 0);
    assert(data);
    switch (type)
    {
        case IM_DT_INT8:
        {
            uint8_t s_buf[4] = {(uint8_t)(color.r * UINT8_MAX), 
                                (uint8_t)(color.g * UINT8_MAX),
                                (uint8_t)(color.b * UINT8_MAX),
                                (uint8_t)(color.a * UINT8_MAX)};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_INT16:
        {
            uint16_t s_buf[4] = {(uint16_t)(color.r * UINT16_MAX), 
                                 (uint16_t)(color.g * UINT16_MAX),
                                 (uint16_t)(color.b * UINT16_MAX),
                                 (uint16_t)(color.a * UINT16_MAX)};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_INT32:
        {
            uint32_t s_buf[4] = {(uint32_t)(color.r * UINT32_MAX), 
                                 (uint32_t)(color.g * UINT32_MAX),
                                 (uint32_t)(color.b * UINT32_MAX),
                                 (uint32_t)(color.a * UINT32_MAX)};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_INT64:
        {
            uint64_t s_buf[4] = {(uint64_t)(color.r * UINT64_MAX), 
                                 (uint64_t)(color.g * UINT64_MAX),
                                 (uint64_t)(color.b * UINT64_MAX),
                                 (uint64_t)(color.a * UINT64_MAX)};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_FLOAT32:
        {
            float s_buf[4] = {color.r, color.g, color.b, color.a};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_FLOAT64:
        {
            double s_buf[4] = {(double)color.r, (double)color.g, (double)color.b, (double)color.a};
            for (int i = 0; i < total() / c; i++)
            {
                memcpy((uint8_t*)data + i * elemsize * c, s_buf, c * elemsize);
            }
        }
        break;
        case IM_DT_FLOAT16:
        // TODO::Dicky add float16 clean with ImPixel
        break;
        default: break;
    }
}

void ImMat::print(std::string name)
{
    std::cout << name << std::endl << "[" << std::endl;
    if (dims == 1)
    {
        for (int _w = 0; _w < w; _w++)
        {
            switch(type)
            {
                case IM_DT_INT8:    std::cout << (int)at<int8_t> (_w) << " "; break;
                case IM_DT_INT16:   std::cout << at<int16_t>(_w) << " "; break;
                case IM_DT_INT32:   std::cout << at<int32_t>(_w) << " "; break;
                case IM_DT_INT64:   std::cout << at<int64_t>(_w) << " "; break;
                case IM_DT_FLOAT32: std::cout << at<float>  (_w) << " "; break;
                case IM_DT_FLOAT64: std::cout << at<double> (_w) << " "; break;
                case IM_DT_FLOAT16: std::cout << im_float16_to_float32(at<uint16_t>  (_w)) << " "; break;
                default: break;
            }
        }
    }
    else if (dims == 2)
    {
        for (int _h = 0; _h < h; _h++)
        {
            std::cout << "    [ ";
            for (int _w = 0; _w < w; _w++)
            {
                switch(type)
                {
                    case IM_DT_INT8:    std::cout << (int)at<int8_t> (_w, _h) << " "; break;
                    case IM_DT_INT16:   std::cout << at<int16_t>(_w, _h) << " "; break;
                    case IM_DT_INT32:   std::cout << at<int32_t>(_w, _h) << " "; break;
                    case IM_DT_INT64:   std::cout << at<int64_t>(_w, _h) << " "; break;
                    case IM_DT_FLOAT32: std::cout << at<float>  (_w, _h) << " "; break;
                    case IM_DT_FLOAT64: std::cout << at<double> (_w, _h) << " "; break;
                    case IM_DT_FLOAT16: std::cout << im_float16_to_float32(at<uint16_t>  (_w, _h)) << " "; break;
                    default: break;
                }
            }
            std::cout << "]" << std::endl;
        }
    }
    else if (dims == 3)
    {
        for (int _c = 0; _c < c; _c++)
        {
            std::cout << "  [ " << std::endl;
            for (int _h = 0; _h < h; _h++)
            {
                std::cout << "    [ ";
                for (int _w = 0; _w < w; _w++)
                {
                    switch(type)
                    {
                        case IM_DT_INT8:    std::cout << at<int8_t> (_w, _h, _c) << " "; break;
                        case IM_DT_INT16:   std::cout << at<int16_t>(_w, _h, _c) << " "; break;
                        case IM_DT_INT32:   std::cout << at<int32_t>(_w, _h, _c) << " "; break;
                        case IM_DT_INT64:   std::cout << at<int64_t>(_w, _h, _c) << " "; break;
                        case IM_DT_FLOAT32: std::cout << at<float>  (_w, _h, _c) << " "; break;
                        case IM_DT_FLOAT64: std::cout << at<double> (_w, _h, _c) << " "; break;
                        case IM_DT_FLOAT16: std::cout << im_float16_to_float32(at<uint16_t>  (_w, _h, _c)) << " "; break;
                        default: break;
                    }
                }
                std::cout << "]" << std::endl;
            }
            std::cout << "  ]" << std::endl;
        }
    }
    
    std::cout << "]" << std::endl;
}

} // namespace ImGui
