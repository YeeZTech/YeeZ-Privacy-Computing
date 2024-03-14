const ByteBuffer = require('bytebuffer')

const BlockFile = function(MagicNumber, BlockNumLimit, ItemNumPerBlockLimit) {
  if (!(this instanceof BlockFile)) {
    return new BlockFile(MagicNumber, BlockNumLimit, ItemNumPerBlockLimit)
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

  let m_header = new header_t(0, 0, 0, 0);
  let m_block_infos = [];
  let block_start_offset = 32 + 32 * BlockNumLimit;

  this.append_item = function(buf, header, block_meta_info) {
    m_header = header;
    m_block_infos = block_meta_info;

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
      if (bi.end_item_index - bi.start_item_index >= ItemNumPerBlockLimit) {
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
    return [m_header, m_block_infos]
  }

}

module.exports = BlockFile
