#
# MIT License
#
# Copyright (c) 2018 drvcoin
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# =============================================================================
#

cmake_minimum_required(VERSION 3.1)

project(libkad)

set(ROOT ${PROJECT_SOURCE_DIR}/../..)

include(${ROOT}/Config.cmake)

add_library(
  kad STATIC

	Action.cpp
	AsyncResult.cpp
	Bucket.cpp
	Buffer.cpp
	BufferedInputStream.cpp
	BufferedOutputStream.cpp
	Config.cpp
	Timer.cpp
	Digest.cpp
	EventLoop.cpp
	FindNodeAction.cpp
	FindValueAction.cpp
  QueryAction.cpp
	IndexQuery.cpp
	IInputStream.cpp
	InstructionSerializer.cpp
	IOutputStream.cpp
	Kademlia.cpp
	KBuckets.cpp
	Key.cpp
	LinuxFileTransport.cpp
	TcpTransport.cpp
	Package.cpp
	PackageDispatcher.cpp
	PingAction.cpp
	Storage.cpp
	StoreAction.cpp
	Thread.cpp
	TransportFactory.cpp
	protocol/FindNode.cpp
	protocol/FindNodeResponse.cpp
	protocol/FindValue.cpp
	protocol/FindValueResponse.cpp
	protocol/Ping.cpp
	protocol/Pong.cpp
	protocol/Store.cpp
	protocol/StoreResponse.cpp
  protocol/Query.cpp
  protocol/QueryResponse.cpp
)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ROOT_DRIVE}/src/jsoncpp/include)

