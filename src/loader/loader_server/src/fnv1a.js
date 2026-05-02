
exports.prime86 = 0x811C9DC5;
exports.FNV1A = function(buf){
  if (typeof buf === "string")
    buf = Buffer.from(buf, 'utf8');

  let ret = 0x811C9DC5;

  for(var i = 0; i < buf.length; i++){
    ret ^= buf[i];
    ret += (ret << 1) + (ret << 4) + (ret << 7) + (ret << 8) + (ret << 24);
  }

  return ret >>> 0;
}

exports.FNV1A_64 = function(buf) {
  if (typeof buf === "string")
    buf = Buffer.from(buf, 'utf8');

  let ret = 0xcbf29ce484222325n;
  const prime = 0x100000001b3n;

  for (let i = 0; i < buf.length; i++) {
    ret ^= BigInt(buf[i]);
    ret *= prime;
    ret &= 0xFFFFFFFFFFFFFFFFn;
  }

  return ret;
}
