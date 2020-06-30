-- an implementation of printf

function printf(...)
 io.write(string.format(...))
end

printf("Hello %s from %s\n",os.getenv"USER" or "there",_VERSION)
