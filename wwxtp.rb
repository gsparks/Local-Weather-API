class Wwxtp < Sinatra::Base

  # This is the main file for a Sinatra app.
  # The two routines below are controllers that specify what to do when presented with a GET or POST request from the index page.

  # Extracts elements from server xml.
  def parse(xml)
    doc = Hpricot::XML(xml)
    @position = Array.new
    @position[0] = doc.at("//position")['lat']
    @position[1] = doc.at("//position")['lon']
    @temp = (doc.at("//temp/*") << doc.at("//temp")['units'])
    @windspeed = (doc.at("//wind-speed/*") << doc.at("//wind-speed")['units'])
    @winddirection = (doc.at("//wind-direction/*") << " degrees")
  end

  # Get / => serve index page.
  # Slim is a markup language like haml.
  get "/" do
    slim :index
  end

  # Post takes in the user request and server (if specified) and saves them to the params hash.
  # client.c is then involked with the appropriate amount of arguments using popen to run it and save the output in a ruby object.
  post '/' do
    @server = params[:server]
    @fake = "<wwxtp>
	          <response>
		       <position lat=\"34.4261944\" lon=\"-119.8415000\" />
		       <temp units=\"F\">91.0</temp>
		       <wind-speed units=\"m/s\">1.9</wind-speed>
		       <wind-direction>231.1</wind-direction>
	          </response>
	         </wwxtp>"
    @request = "<wwxtp><request><command>TEST</command><position lat=\"y\" lon=\"z\" /></request></wwxtp>".gsub(/[yz]/, 'y' => "#{params[:request].split(" ")[0]}", 'z' => "#{params[:request].split(" ")[1]}")		# Use string substitution to create xml from user input.
    if params[:server] == ""				# branch to execute if there is no specified server.
      output = IO.popen('./client "request"'.gsub(/request/, "#{@request}"))    # output of running c client.
      parse(@fake)
    else
      output = IO.popen('./client "r" "s"'.gsub(/[rs]/, 'r' => "#{@request}", 's' => "#{@server}")).to_s
      parse(@fake)
    end
    slim :input
  end

end
