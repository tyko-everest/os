#[repr(C, align(8))]
struct header {
    name: [u8; 100],
    mode: [u8; 8],
    uid: [u8; 8],
    gid: [u8; 8],
    size: [u8; 12],
    time: [u8; 12],
    checksum: [u8; 12],
    file_type: u8,
    link_name: [u8; 100],
    padding: [u8; 255],
}
