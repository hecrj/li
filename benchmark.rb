`g++ main.cpp -O3`

tests = []

5.times do |i|
  10.times do |j|
    tests << "#{(i+2)*50}-#{j+1}"
  end
end

tests.each do |file|
#    start = Time.now()
     output = `./a.out < test/vars-#{file}.cnf`
     
    puts output[/(UN)?SATISFIABLE/]

     
#    time1 = Time.now() - start

#    start = Time.now()
#    `picosat < #{file}`
#    time2 = Time.now() - start
    
#    puts "#{file.gsub("test/vars-", "").gsub(".cnf", "").ljust(20)} #{time1.round(3).to_s.ljust(10)} #{time2.round(3)}"
end
