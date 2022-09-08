var ByteBuffer = require('bytebuffer')

const YPCNtObject = function () {
	if (!(this instanceof YPCNtObject)) {
		return new YPCNtObject()
	}

	//input should be an array, which like
	//[{"type":"bool", "value": true},
	// {"type":"int8", "value":120},
	// {"type":"bytes", "value":0xdadfdfdfdfd}]
	this.generateBytes = function (input) {
		let length = this.getLengthOf(input)
		let buffer = new ByteBuffer(length, ByteBuffer.LITTLE_ENDIAN)
		var inputLength = input.length
		let offset = 4
		for (var i = 0; i < inputLength; i++) {
			let d = input[i]
			let v = d['value']

			switch (d['type']) {
				case 'bool':
					buffer.writeInt8(d['value'] == true ? 1 : 0, offset)
					offset += 1
					break
				case 'uint8_t':
					buffer.writeUint8(v, offset)
					offset += 1
					break
				case 'int8_t':
					buffer.writeInt8(v, offset)
					offset += 1
					break
				case 'int16_t':
					buffer.writeInt16(v, offset)
					offset += 2
					break
				case 'uint16_t':
					buffer.writeUint16(v, offset)
					offset += 2
					break
				case 'int32_t':
					buffer.writeInt32(v, offset)
					offset += 4
					break
				case 'uint32_t':
					buffer.writeUint32(v, offset)
					offset += 4
					break
				case 'int64_t':
					buffer.writeInt64(v, offset)
					offset += 8
					break
				case 'uint64_t':
					buffer.writeUint16(v, offset)
					offset += 8
					break
				case 'float':
					buffer.writeFloat(v, offset)
					offset += 4
					break
				case 'double':
					buffer.writeDouble(v, offset)
					offset += 8
					break
				case 'string':
				  byteLen = Buffer.byteLength(v, 'utf8')
					buffer.writeUint64(byteLen, offset)
					offset += 8
					buffer.writeString(v, offset)
					offset += byteLen
					break
				case 'bytes':
					buffer.writeUint64(v.length, offset)
					offset += 8
					buffer.append(v, offset)
					offset += v.length
					break
			}
		}
		return buffer
	}

	// schema = [{"name": "", "type": "string"}, {"name": "", "type": "uint64_t"}]
	this.decodeBytes = function (schema, buf) {
		if(!schema) return ''
		const buffer = new ByteBuffer(buf.length, ByteBuffer.LITTLE_ENDIAN);
		buffer.append(buf, 0);
		let offset = 4;
		let args = [];
		let v = null;
		for (let i = 0; i < schema.length; i++) {
			let d = schema[i];
			args.push(d);
			switch (d["type"]) {
				case "bool":
					v = buffer.readInt8(offset);
					args[i]["value"] = v == 1 ? true : false;
					offset += 1;
					break;
				case "int8_t":
					v = buffer.readInt8(offset);
					args[i]["value"] = v;
					offset += 1;
					break;
				case "uint8_t":
					v = buffer.readUint8(offset);
					args[i]["value"] = v;
					offset += 1;
					break;
				case "int16_t":
					v = buffer.readInt16(offset);
					args[i]["value"] = v;
					offset += 2;
					break;
				case "uint16_t":
					v = buffer.readUint16(offset);
					args[i]["value"] = v;
					offset += 2;
					break;
				case "int32_t":
					v = buffer.readInt32(offset);
					args[i]["value"] = v;
					offset += 4;
					break;
				case "uint32_t":
					v = buffer.readUint32(offset);
					args[i]["value"] = v;
					offset += 4;
					break;
				case "int64_t":
					v = buffer.readInt64(offset);
					args[i]["value"] = v.toNumber();
					offset += 8;
					break;
				case "uint64_t":
					v = buffer.readUint64(offset);
					args[i]["value"] = v.toNumber();
					offset += 8;
					break;
				case "float":
					v = buffer.readFloat(offset);
					args[i]["value"] = v;
					offset += 4;
					break;
				case "double":
					v = buffer.readDouble(offset);
					args[i]["value"] = v;
					offset += 8;
					break;
				case "string":
					byteLen = buffer.readUint64(offset);
					byteLen = byteLen.toNumber();
					offset += 8;
					buf = buffer.slice(offset, offset + byteLen);
					//v = buffer.readString(byteLen, offset)
					args[i]["value"] = buf.toString("utf8");
					offset += byteLen;
					break;
			}
		}
		return args;
	};

	this.getLengthOf = function (input) {
		var c = 4
		var inputLength = input.length
		for (var i = 0; i < inputLength; i++) {
			let d = input[i]
			let l = 0
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
				  byteLen = Buffer.byteLength(d['value'], 'utf8')
					l = 8 + byteLen
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
