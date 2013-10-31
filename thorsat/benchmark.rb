`g++ main.cpp -O3 -DVERBOSE`

tests = []

5.times do |i|
  10.times do |j|
    tests << "#{(i+2)*50}-#{j+1}"
  end
end

decisions = []
conflicts = []
propagations = []

tests.each do |file|
start = Time.now()
    output = `./a.out < test/vars-#{file}.cnf`
     
    decisions << output[/\d+ decisions/].gsub(" decisions", "")
    conflicts << output[/\d+ conflicts/].gsub(" conflicts", "")
    propagations << output[/\d+ propagations/].gsub(" propagations", "")
     
    # puts (Time.now() - start).round(3)

#    start = Time.now()
#    `picosat < #{file}`
#    time2 = Time.now() - start
    
#    puts "#{file.gsub("test/vars-", "").gsub(".cnf", "").ljust(20)} #{time1.round(3).to_s.ljust(10)} #{time2.round(3)}"
end

puts "Decisions:"
decisions.each { |d| puts d }
puts ""

puts "Conflicts:"
conflicts.each { |c| puts c }
puts ""

puts "Propagations:"
propagations.each { |p| puts p }
puts ""
