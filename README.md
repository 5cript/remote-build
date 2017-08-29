# remote-build
DO NOT USE, ITS NOT SAFE.

## Program Options
```
Allowed options:
  --help                               produce help message
  -i [ --config ] arg (=./config.json) config to use
  -m [ --make-directories ]            create directory on remote host
  -u [ --upload ]                      upload files to host
  -b [ --build ]                       build stuff on host
  -c [ --clean ]                       clean stuff on host
  -o [ --updated-only ]                clean stuff on host
  -t [ --timeout ] arg (=60)           build wait timeout (seconds)
```

## Example Confgi
```JSON
{
  "config": {
	"serverAddress": "192.168.168.96",
	"local": "D:/projects/list-disk",
	"id": "list-disk",
	"log": "./output.txt",	
	"user": "admin",
	"password": "admin",
	"globExpression": "*.?pp"
  }
}
```
