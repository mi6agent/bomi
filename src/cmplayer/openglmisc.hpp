#ifndef OPENGLMISC_HPP
#define OPENGLMISC_HPP

#include "stdafx.hpp"

#ifdef PixelFormat
#undef PixelFormat
#endif

typedef QOpenGLTexture OGL;

static constexpr OGL::PixelType OGL_UInt32_RGBA8 = (OGL::PixelType)GL_UNSIGNED_INT_8_8_8_8;
static constexpr OGL::PixelType OGL_UInt32_RGBA8_Rev = (OGL::PixelType)GL_UNSIGNED_INT_8_8_8_8_REV;
static constexpr OGL::PixelType OGL_UInt16_Apple = (OGL::PixelType)GL_UNSIGNED_SHORT_8_8_APPLE;
static constexpr OGL::PixelType OGL_UInt16_Rev_Apple = (OGL::PixelType)GL_UNSIGNED_SHORT_8_8_REV_APPLE;
static constexpr OGL::PixelType OGL_UInt16_Mesa = (OGL::PixelType)GL_UNSIGNED_SHORT_8_8_MESA;
static constexpr OGL::PixelType OGL_UInt16_Rev_Mesa = (OGL::PixelType)GL_UNSIGNED_SHORT_8_8_REV_MESA;

static constexpr OGL::TextureFormat OGL_Luminance8_UNorm = (OGL::TextureFormat)GL_LUMINANCE8;
static constexpr OGL::TextureFormat OGL_Luminance16_UNorm = (OGL::TextureFormat)GL_LUMINANCE16;
static constexpr OGL::TextureFormat OGL_LuminanceAlpha8_UNorm = (OGL::TextureFormat)GL_LUMINANCE8_ALPHA8;
static constexpr OGL::TextureFormat OGL_LuminanceAlpha16_UNorm = (OGL::TextureFormat)GL_LUMINANCE16_ALPHA16;
static constexpr OGL::TextureFormat OGL_YCbCr_UNorm_Mesa = (OGL::TextureFormat)GL_YCBCR_MESA;

static constexpr OGL::PixelFormat OGL_YCbCr_422_Apple = (OGL::PixelFormat)GL_YCBCR_422_APPLE;
static constexpr OGL::PixelFormat OGL_YCbCr_Mesa = (OGL::PixelFormat)GL_YCBCR_MESA;


struct OpenGLTextureFormat {
	OpenGLTextureFormat() {}
	OpenGLTextureFormat(OGL::TextureFormat internal, OGL::PixelFormat pixel, OGL::PixelType type)
	: internal(internal), pixel(pixel), type(type) {}
	OGL::TextureFormat internal = OGL::NoFormat;
	OGL::PixelFormat pixel = OGL::NoSourceFormat;
	OGL::PixelType type = OGL::NoPixelType;
};

class OpenGLTexture {
public:
	virtual ~OpenGLTexture() = default;
	GLuint id = GL_NONE;
	OGL::Target target = OGL::Target2D;
	int width = 0, height = 0, depth = 0;
	OpenGLTextureFormat format;
	QSize size() const { return {width, height}; }
	void copyAttributesFrom(const OpenGLTexture &other) {
		target = other.target;
		width = other.width;
		height = other.height;
		depth = other.depth;
		format = other.format;
	}
	void setSize(const QSize &size) { width = size.width(); height = size.height(); }
	void generate() { glGenTextures(1, &id); }
	void delete_() { glDeleteTextures(1, &id); }
	void bind() const { glBindTexture(target, id); }
	bool allocate(const void *data = nullptr) const {
		return allocate(OGL::Linear, OGL::ClampToEdge, data);
	}
	bool expand(const QSize &size, double mul = 1.2) {
		if (width >= size.width() && height >= size.height())
			return true;
		if (width < size.width())
			width = size.width()*mul;
		if (height < size.height())
			height = size.width()*mul;
		return allocate();
	}
	bool allocate(int filter, int clamp, const void *data = nullptr) const {
		if (!width && !height && !depth)
			return false;
		bind();
		switch (target) {
		case GL_TEXTURE_3D:
			glTexImage3D(target, 0, format.internal, width, height, depth, 0, format.pixel, format.type, data);
			break;
		case OGL::Target2D:
		case OGL::TargetRectangle:
			glTexImage2D(target, 0, format.internal, width, height, 0, format.pixel, format.type, data);
			break;
		case OGL::Target1D:
			glTexImage1D(target, 0, format.internal, width, 0, format.pixel, format.type, data);
			break;
		default:
			return false;
		}
		glTexParameterf(target, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameterf(target, GL_TEXTURE_MIN_FILTER, filter);
		switch (target) {
		case GL_TEXTURE_3D:
			glTexParameterf(target, GL_TEXTURE_WRAP_R, clamp);
		case OGL::Target2D:
		case OGL::TargetRectangle:
			glTexParameterf(target, GL_TEXTURE_WRAP_T, clamp);
		case OGL::Target1D:
			glTexParameterf(target, GL_TEXTURE_WRAP_S, clamp);
			break;
		default:
			return false;
		}
		unbind();
		return true;
	}
	bool upload(const void *data) const {
		switch (target) {
		case GL_TEXTURE_3D:
			return upload(0, 0, 0, width, height, depth, data);
		case OGL::Target2D:
		case OGL::TargetRectangle:
			return upload(0, 0, width, height, data);
		case OGL::Target1D:
			return upload(0, width, data);
		default:
			return false;
		}
	}
	bool isNull() const { return id == GL_NONE; }
	bool isEmpty() const { return !width && !height && !depth; }
	bool upload1D(const void *data) const { return upload(0, width, data); }
	bool upload2D(const void *data) const { return upload(0, 0, width, height, data); }
	bool upload(int x, int y, int z, int width, int height, int depth, const void *data) const {
		if (isEmpty())
			return false;
		bind();
		glTexSubImage3D(target, 0, x, y, z, width, height, depth, format.pixel, format.type, data);
		unbind();
		return true;
	}
	bool upload(int x, int y, int width, int height, const void *data) const {
		if (isEmpty())
			return false;
		bind();
		glTexSubImage2D(target, 0, x, y, width, height, format.pixel, format.type, data);
		unbind();
		return true;
	}
	bool upload(int x, int y, const QSize &size, const void *data) const {
		return upload(x, y, size.width(), size.height(), data);
	}
	bool upload(const QPoint &pos, const QSize &size, const void *data) const {
		return upload(pos.x(), pos.y(), size.width(), size.height(), data);
	}
	bool upload(int x, int width, const void *data) const {
		if (isEmpty())
			return false;
		bind();
		glTexSubImage1D(target, 0, x, width, format.pixel, format.type, data);
		unbind();
		return true;
	}
	void unbind() const { glBindTexture(target, 0); }
	QImage toImage() const;
};

class OpenGLFramebufferObject : public QOpenGLFramebufferObject {
public:
	OpenGLFramebufferObject(const QSize &size, QOpenGLTexture::TextureFormat internal = QOpenGLTexture::RGBA8_UNorm)
	: QOpenGLFramebufferObject(size, NoAttachment, OGL::Target2D, internal) {
		m_texture.id = QOpenGLFramebufferObject::texture();
		m_texture.width = size.width();
		m_texture.height = size.height();
		m_texture.target = OGL::Target2D;
		m_texture.format.internal = internal;
		m_texture.format.pixel = OGL::RGBA;
		m_texture.format.type = OGL::UInt8;
		if (isValid()) {
			m_texture.bind();
			glTexParameterf(m_texture.target, GL_TEXTURE_MAG_FILTER, OGL::Linear);
			glTexParameterf(m_texture.target, GL_TEXTURE_MIN_FILTER, OGL::Linear);
			m_texture.unbind();
		}
	}
	const OpenGLTexture &texture() const { return m_texture; }
	void getCoords(double &x1, double &y1, double &x2, double &y2) {
		if (m_texture.target == OGL::TargetRectangle) {
			x1 = y1 = 0; x2 = m_texture.width; y2 = m_texture.height;
		} else { x1 = y1 = 0; x2 = y2 = 1; }
	}
	QImage toImage() const;
private:
	OpenGLTexture m_texture;
};

class OpenGLTextureShaderProgram : public QOpenGLShaderProgram {
	enum {vPosition, vCoord, vColor};
public:
	static constexpr int N = 6;
	OpenGLTextureShaderProgram(QObject *parent = nullptr): QOpenGLShaderProgram(parent) { }
	void setFragmentShader(const QByteArray &code) {
		if (!m_frag)
			m_frag = addShaderFromSourceCode(QOpenGLShader::Fragment, code);
	}
	void setVertexShader(const QByteArray &code) {
		if (!m_vertex) {
			m_vertex = addShaderFromSourceCode(QOpenGLShader::Vertex, code);
			m_hasColor = code.contains("vColor");
		}
	}
	bool link() override {
		bindAttributeLocation("vCoord", vCoord);
		bindAttributeLocation("vPosition", vPosition);
		if (m_hasColor)
			bindAttributeLocation("vColor", vColor);
		return QOpenGLShaderProgram::link();
	}
	void setTextureCount(int textures) {
		if (_Expand(m_vPositions, 2*N*textures)) {
			m_vCoords.resize(m_vPositions.size());
			if (m_hasColor)
				m_vColors.resize(m_vPositions.size()/2);
		}
	}
	void uploadPositionAsTriangles(int i, const QPointF &p1, const QPointF &p2) {
		uploadRectAsTriangles(m_vPositions.data(), i, p1, p2);
	}
	void uploadPositionAsTriangles(int i, const QRectF &rect) {
		uploadRectAsTriangles(m_vPositions.data(), i, rect.topLeft(), rect.bottomRight());
	}
	void uploadCoordAsTriangles(int i, const QPointF &p1, const QPointF &p2) {
		uploadRectAsTriangles(m_vCoords.data(), i, p1, p2);
	}
	void uploadCoordAsTriangles(int i, const QRectF &rect) {
		uploadRectAsTriangles(m_vCoords.data(), i, rect.topLeft(), rect.bottomRight());
	}
	void uploadColorAsTriangles(int i, quint32 color) {
		auto p = m_vColors.data() + N*i;
		*p++ = color; *p++ = color; *p++ = color; *p++ = color; *p++ = color; *p++ = color;
	}
	void begin() {
		bind();
		enableAttributeArray(vPosition);
		enableAttributeArray(vCoord);
		setAttributeArray(vCoord, m_vCoords.data(), 2);
		setAttributeArray(vPosition, m_vPositions.data(), 2);
		if (m_hasColor) {
			enableAttributeArray(vColor);
			setAttributeArray(vColor, OGL::UInt8, m_vColors.data(), 4);
		}
	}
	void end() {
		disableAttributeArray(vCoord);
		disableAttributeArray(vPosition);
		if (m_hasColor)
			disableAttributeArray(vColor);
		release();
	}
	void reset() { removeAllShaders(); m_frag = m_vertex = false; }
private:
	void uploadRectAsTriangles(float *p, int i, const QPointF &p1, const QPointF &p2) {
		p += N*2*i;
		*p++ = p1.x(); *p++ = p1.y();
		*p++ = p2.x(); *p++ = p1.y();
		*p++ = p1.x(); *p++ = p2.y();

		*p++ = p1.x(); *p++ = p2.y();
		*p++ = p2.x(); *p++ = p2.y();
		*p++ = p2.x(); *p++ = p1.y();
	}
	QVector<float> m_vPositions, m_vCoords;
	QVector<quint32> m_vColors;
	bool m_hasColor = false, m_frag = false, m_vertex = false;
};

#endif // OPENGLMISC_HPP