const ByteBuffer = require('bytebuffer')
const keccak256 = require('keccak256')
const YPCCrypto = require('./ypccrypto.js')()
const YPCNtObject = require('./ypcntobject.js')()

const BlockNumLimit = 1024 * 1024
const max_item_size = 64 * 1024
const BlockFile = require('./blockfile.js')(
	Buffer.from('1fe2ef7f3ed18847', 'hex'),
	BlockNumLimit,
	256 * max_item_size
)

const DataProvider = function () {
	if (!(this instanceof DataProvider)) {
		return new DataProvider()
	}

	// 32 bytes
	function header_t(magic_number, version_number, block_number, item_number) {
		this.magic_number = magic_number
		this.version_number = version_number
		this.block_number = block_number
		this.item_number = item_number
	}

	function header_t2buffer(header) {
		let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN)
		buf.append(header.magic_number, 0)
		buf.writeUint64(header.version_number, 8)
		buf.writeUint64(header.block_number, 16)
		buf.writeUint64(header.item_number, 24)
		return buf
	}

	function block_info_t2buffer(bi) {
		let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN)
		buf.writeUint64(bi.start_item_index, 0)
		buf.writeUint64(bi.end_item_index, 8)
		buf.writeUint64(bi.start_file_pos, 16)
		buf.writeUint64(bi.end_file_pos, 24)
		return buf
	}

	let header = new header_t(0, 0, 0, 0)
	let block_meta_info = []
	//let sealed_data = new Uint8Array(0)
	let sealed_data = []

	function batch2ntpackage(batch) {
		buf_size = 4 + 8
		for (let i = 0; i < batch.length; i++) {
			buf_size += 8
			buf_size += batch[i].length
		}

		let buf = new ByteBuffer(buf_size, ByteBuffer.LITTLE_ENDIAN)
		offset = 0
		// package id
		buf.writeUint32(0x82c4e8d8, offset)
		offset += 4
		// batch size
		buf.writeUint64(batch.length, offset)
		offset += 8
		// batch items
		for (let i = 0; i < batch.length; i++) {
			buf.writeUint64(batch[i].length, offset)
			offset += 8
			buf.append(batch[i], offset)
			offset += batch[i].length
		}
		return buf.buffer
	}

	function write_batch(batch, public_key) {
		let pkg_bytes = batch2ntpackage(batch)
		const ots = YPCCrypto.generatePrivateKey()
		let s = YPCCrypto._encryptMessage(
		  Buffer.from(public_key, 'hex'),
		  ots,
		  pkg_bytes,
		  0x2
		)
		let all = BlockFile.append_item(s, header, block_meta_info)
		header = all[0]
		block_meta_info = all[1]
		let data = new ByteBuffer(8 + s.length,ByteBuffer.LITTLE_ENDIAN)
		data.writeUint64(s.length, 0)
		data.append(s, 8)
		sealed_data.push(data.buffer)
	}

	function ignore_leading_and_tailing_spaces(item) {
		let idx = 0
		while (item[idx] === ' ' || item[idx] === '\t' || item[idx] === '\r') {
			idx++
		}
		item = item.substring(idx)
		idx = item.length - 1
		while (item[idx] === ' ' || item[idx] === '\t' || item[idx] === '\r') {
			idx--
		}
		item = item.substring(0, idx + 1)
		return item
	}

	let all_line_num = 0,
		now_line_num = 0

	this.sealPercent = function () {
		if (all_line_num === 0 || now_line_num === 0) {
			return 0
		}
		return (100.0 * now_line_num) / all_line_num
	}

  const anyEnclave = Buffer.from('bd0c3cce561fac62b90ddd7bfcfe014702aa4327bc2b0b69ef79a7d2a0350f11', 'hex');
	let data_lines = []
	this.init = function (_data_lines) {
	  let line;
    while (line = _data_lines.next()) {
      data_lines.push(line);
    }
	}

	this.sealFile = function (key_file) {
		all_line_num = data_lines.length

		let data_hash = keccak256(Buffer.from('Fidelius', 'utf-8'))
		let line,
			counter = 0
		let batch = [],
			batch_size = 0

		for (let l = 0; l < data_lines.length; l++) {
			line = data_lines[l]
			//line = line.toString().replace('\r', '').replace('\t', '')
			let items = line.toString().split(',')
			let input = []
			for (let i = 0; i < items.length; i++) {
				let item = ignore_leading_and_tailing_spaces(items[i])
				let obj = { type: 'string', value: item }
				input.push(obj)
			}
			let ntInput = YPCNtObject.generateBytes(input)
			batch.push(ntInput.buffer)
			batch_size += ntInput.buffer.length
			if (batch_size >= max_item_size) {
				write_batch(batch, key_file['public-key'])
				batch = []
				batch_size = 0
			}
			let k = Buffer.from(
				data_hash.toString('hex') + Buffer.from(ntInput.buffer).toString('hex'),
				'hex'
			)
			data_hash = keccak256(k)
			counter++
			now_line_num = counter
		}
		console.log('data hash:', data_hash.toString('hex'))

		if (batch.length != 0) {
			write_batch(batch, key_file['public-key'])
		}

    let sealed_data_size = 0;
    for (let i = 0; i < sealed_data.length; i++) {
      sealed_data_size += sealed_data[i].length;
    }
		let block_start_offset = 32 + 32 * BlockNumLimit
		let all = new ByteBuffer(
			block_start_offset + sealed_data_size,
			ByteBuffer.LITTLE_ENDIAN
		)
		let offset = 0
		// set header
		let buf_header = header_t2buffer(header)
		all.append(buf_header, offset)
		offset += 32
		// set block meta
		for (let i = 0; i < header.block_number; i++) {
			let bi = block_meta_info[i]
			let buf_bi = block_info_t2buffer(bi)
			all.append(buf_bi, offset)
			offset += 32
		}
		// set data
		let data_offset = 0;
    for (let i = 0; i < sealed_data.length; i++) {
      all.append(sealed_data[i], block_start_offset + data_offset);
      data_offset += sealed_data[i].length;
    }

    let output_content = '';
    output_content += ('public_key = ' + key_file['public-key'] + '\n');
    output_content += ('data_id = ' + data_hash.toString('hex') + '\n');
    output_content += ('item_num = ' + counter + '\n');

		return [all, output_content]
	}

}

module.exports = DataProvider

