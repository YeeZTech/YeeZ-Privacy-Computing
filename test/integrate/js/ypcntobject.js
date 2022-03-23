var ByteBuffer = require('bytebuffer')
var Long = require('long')

const YPCNtObject = function () {
	if (!(this instanceof YPCNtObject)) {
		return new YPCNtObject()
	}

	//input should be an array, which like
	//[{"type":"bool", "value": true},
	// {"type":"int8", "value":120},
	// {"type":"bytes", "value":0xdadfdfdfdfd}]
	this.generateBytes = function (input) {
		length = this.getLengthOf(input)
		buffer = new ByteBuffer(length, ByteBuffer.LITTLE_ENDIAN)
		var inputLength = input.length
		offset = 4
		for (var i = 0; i < inputLength; i++) {
			d = input[i]
			v = d['value']
			switch (d['type']) {
				case 'bool':
					buffer.writeInt8(d['value'] === 'true' ? 1 : 0, offset)
					offset += 1
					break
				case 'uint8_t':
					buffer.writeUint8(Number(v), offset)
					offset += 1
					break
				case 'int8_t':
					buffer.writeInt8(Number(v), offset)
					offset += 1
					break
				case 'int16_t':
					buffer.writeInt16(Number(v), offset)
					offset += 2
					break
				case 'uint16_t':
					buffer.writeUint16(Number(v), offset)
					offset += 2
					break
				case 'int32_t':
					buffer.writeInt32(Number(v), offset)
					offset += 4
					break
				case 'uint32_t':
					buffer.writeUint32(Number(v), offset)
					offset += 4
					break
				case 'int64_t':
					buffer.writeInt64(ByteBuffer.Long.fromString(v, false, 10), offset)
					offset += 8
					break
				case 'uint64_t':
					buffer.writeUint64(ByteBuffer.Long.fromString(v, true, 10), offset)
					offset += 8
					break
				case 'float':
					buffer.writeFloat(Number(v), offset)
					offset += 4
					break
				case 'double':
					buffer.writeDouble(Number(v), offset)
					offset += 8
					break
				case 'string':
					buffer.writeUint64(Buffer.byteLength(v, 'utf8'), offset)
					offset += 8
					buffer.writeString(v, offset)
					offset += v.length
					break
				case 'bytes':
					buffer.writeUint64(Buffer.from(v, 'utf-8').length, offset)
					offset += 8
					buffer.append(v, offset)
					offset += v.length
					break
			}
		}
		return buffer
	}

	this.getLengthOf = function (input) {
		var c = 4
		var inputLength = input.length
		for (var i = 0; i < inputLength; i++) {
			d = input[i]
			l = 0
			switch (d['type']) {
				case 'bool':
					l = 1
					break
				case 'uint8_t':
					l = 1
					break
				case 'int8_t':
					l = 1
					break
				case 'int16_t':
					l = 2
					break
				case 'uint16_t':
					l = 2
					break
				case 'int32_t':
					l = 4
					break
				case 'uint32_t':
					l = 4
					break
				case 'int64_t':
					l = 8
					break
				case 'uint64_t':
					l = 8
					break
				case 'float':
					l = 4
					break
				case 'double':
					l = 8
					break
				case 'string':
					l = 8 + d['value'].length
					break
				case 'bytes':
					l = 8 + d['value'].length
					break
			}
			c = c + l
		}
		return c
	}
}

module.exports = YPCNtObject
