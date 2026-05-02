var FNV1A = function(buf){
  let ret = 0x811C9DC5;

  for(var i = 0; i < buf.length; i++){
    ret ^= buf.charCodeAt(i);
    ret += (ret << 1) + (ret << 4) + (ret << 7) + (ret << 8) + (ret << 24);
  }

  return ret >>> 0;
}

console.log(FNV1A("test1234").toString(16));