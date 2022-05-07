const fs = require('fs');
const ByteBuffer = require('bytebuffer')

const BlockFile = function(MagicNumber, BlockNumLimit, BlockSizeLimit) {
	if (!(this instanceof BlockFile)) {
		return new BlockFile(MagicNumber, BlockNumLimit, BlockSizeLimit)
	}

  // 32 bytes
  function header_t(magic_number, version_number, block_number, item_number) {
    this.magic_number = magic_number;
    this.version_number = version_number;
    this.block_number = block_number;
    this.item_number = item_number;
  }

  // 32 bytes
  function block_info_t(start_item_index, end_item_index, start_file_pos, end_file_pos) {
    this.start_item_index = start_item_index;
    this.end_item_index = end_item_index;
    this.start_file_pos = start_file_pos;
    this.end_file_pos = end_file_pos;
  }

  function header_t2buffer(header) {
		let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN)
		buf.append(header.magic_number, 0);
		buf.writeUint64(header.version_number, 8);
		buf.writeUint64(header.block_number, 16);
		buf.writeUint64(header.item_number, 24);
		return buf.buffer;
  }

  function block_info_t2buffer(bi) {
		let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN)
		buf.writeUint64(bi.start_item_index, 0);
		buf.writeUint64(bi.end_item_index, 8);
		buf.writeUint64(bi.start_file_pos, 16);
		buf.writeUint64(bi.end_file_pos, 24);
		return buf.buffer;
  }

	let m_header = new header_t(0, 0, 0, 0);
	let m_block_infos = [];
	let block_start_offset = 32 + 32 * BlockNumLimit;

  this.open_for_write = function(path) {
    return fs.openSync(path, 'w+');
  }

  this.close_file = function(fd) {
    return fs.closeSync(fd);
  }

  function read_header(fd) {
		let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN)
    fs.readSync(fd, buf.buffer, 0, 32, 0);
    m_header.magic_number = buf.buffer.slice(0, 8);
    m_header.version_number = buf.readUint64(8).toNumber();
    m_header.block_number = buf.readUint64(16).toNumber();
    m_header.item_number = buf.readUint64(24).toNumber();
  }
  function read_block_meta_info(fd) {
    read_header(fd);
    m_block_infos = [];
    for (let i = 0; i < m_header.block_number; i++) {
      let buf = new ByteBuffer(32, ByteBuffer.LITTLE_ENDIAN);
      fs.readSync(fd, buf.buffer, 0, 32, 32 + 32 * i);
      bi = new block_info_t(0, 0, 0, 0);
      bi.start_item_index = buf.readUint64(0).toNumber();
      bi.end_item_index = buf.readUint64(8).toNumber();
      bi.start_file_pos = buf.readUint64(16).toNumber();
      bi.end_file_pos = buf.readUint64(24).toNumber();
      m_block_infos.push(bi);
    }
  }

  this.append_item = function(fd, buf) {
    read_block_meta_info(fd);

    m_header.item_number++;
    m_header.magic_number = MagicNumber;

    bi = new block_info_t(0, 0, 0, 0);
    if (0 === m_block_infos.length) {
      bi.start_item_index = 0;
      bi.end_item_index = 1;
      bi.start_file_pos = block_start_offset;
      bi.end_file_pos = bi.start_file_pos + buf.length + 8;
      m_block_infos.push(bi);
      m_header.block_number++;
    } else {
      bi = m_block_infos[m_block_infos.length - 1];
      if (bi.end_item_index - bi.start_item_index >= BlockSizeLimit) {
        new_block = new block_info_t(0, 0, 0, 0);
        new_block.start_item_index = bi.end_item_index;
        new_block.end_item_index = new_block.start_item_index + 1;
        new_block.start_file_pos = bi.end_file_pos;
        new_block.end_file_pos = new_block.start_file_pos + buf.length + 8;
        m_block_infos.push(new_block);
        m_header.block_number++;
      } else {
        bi.end_item_index++;
        bi.end_file_pos = bi.end_file_pos + buf.length + 8;
      }
    }
    // header
    header = header_t2buffer(m_header);
    fs.writeSync(fd, header, 0, header.length, 0);
    // block meta
    pos = 32 + 32 * (m_block_infos.length - 1);
    back = m_block_infos[m_block_infos.length - 1];
    bi = block_info_t2buffer(back);
    fs.writeSync(fd, bi, 0, bi.length, pos);
    // buf length
    pos = back.end_file_pos - buf.length - 8;
		let len = new ByteBuffer(8, ByteBuffer.LITTLE_ENDIAN)
		len.writeUint64(buf.length, 0);
    fs.writeSync(fd, len.buffer, 0, 8, pos);
    // buf
    pos = back.end_file_pos - buf.length;
    fs.writeSync(fd, buf, 0, buf.length, pos);
  }

}

module.exports = BlockFile
