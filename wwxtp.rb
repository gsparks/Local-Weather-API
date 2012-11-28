class Wwxtp < Sinatra::Base

  # This is the main file for a Sinatra app.
  # The two routines below are controllers that specify what to do when presented with a GET or POST request from the index page.

  # Get / => serve index page.
  # Slim is a markup language like haml.
  get "/" do
    slim :index
  end

  # Post takes in the user request and server (if specified) and saves them to the params hash.
  # client.c is then involked with the appropriate amount of arguments using popen to run it and save the output in a ruby object.
  post '/' do
    @server = params[:server]
    @request = "<wwxtp><request><command>TEST</command><position lat=\"y\" lon=\"z\" /></request></wwxtp>".gsub(/[yz]/, 'y' => "#{params[:request].split(" ")[0]}", 'z' => "#{params[:request].split(" ")[1]}")		# Use string substitution to create xml from user input.

    if params[:server] == ""				# branch to execute if there is no specified server.
      output = IO.popen('./client "request"'.gsub(/request/, "#{@request}"))    # output of running c client.
      @output = output.readlines	# read and store output.

    else
      output = IO.popen('./client "r" "s"'.gsub(/[rs]/, 'r' => "#{@request}", 's' => "#{@server}"))
      @output = []
      fake = "<wwxtp>
	<response>
		<position lat=\"34.4261944\" lon=\"-119.8415000\" />
		<temp units=\"F\">91.0</temp>
		<wind-speed units=\"m/s\">1.9</wind-speed>
		<wind-direction>231.1</wind-direction>
	</response>
	</wwxtp>"
      doc = Hpricot::XML(fake)
        (doc/:response).each do |response|
          ['position', 'temp', 'wind-speed', 'wind-direction'].each do |el|
            @output[el] << response.at(el)
          end
        end
    end
    slim :input
  end

end
