##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "boatGrader_lib",
    srcs = [
        "boatGrader.cc",
        ],
    hdrs = [
        "boatGrader.h",
        ],
    copts = [
        # "/std:c++20",
        # "/03",
        "-std=c++2a",
        "-O3"
    ],
	visibility = [
		"//visibility:public",
	],
)

cc_library(
    name = "boat_lib",
    srcs = [
        "boat.cc",
        ],
    hdrs = [
        "boat.h",
        ],
	deps = [
        ":boatGrader_lib"
    ],
    copts = [
        # "/std:c++20",
        # "/03",
        "-std=c++2a",
        "-O3"
    ],
	visibility = [
		"//visibility:public",
	],
)

cc_binary(
    name = "main",
    srcs = [
        "main.cc"
            ],
	deps = [
        ":boatGrader_lib",
		":boat_lib"
    ],
    copts = [
        # "/std:c++20",
        # "/03",
        "-std=c++2a",
        "-O3"
    ],
	linkopts = [
        "-pthread",
    ],
)