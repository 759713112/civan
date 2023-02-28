pushd .
protoc -I. --cpp_out=. civan/store/*.proto
protoc -I. --cpp_out=. civan/*.proto 
popd