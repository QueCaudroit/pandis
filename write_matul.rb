filename = 'tiled_matmul.h'
tile_size = 32

text = "#ifndef TILED_MATMUL\n#define TILED_MATMUL \n\n"
text << "#define TILE_SIZE #{tile_size}\n"
text << "#define TILE_LENGTH #{tile_size * tile_size}\n"
text << "#define TILE_MATMUL(a,b,c) \\\n"

(0...tile_size).each do |i|
  (0...tile_size).each do |j|
    text << "c[#{i*tile_size+j}] += "
    (0...tile_size).each do |k|
      text << "a[#{i*tile_size+k}] * b[#{k*tile_size+j}] + "
    end
    text.chomp!(" + ")
    text << ";\\\n"
  end
end

text.chomp!("\\\n")
text << "\n\n#endif\n"

File.open(filename, 'w') { |file| file.write(text) }

