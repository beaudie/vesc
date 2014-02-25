vars = {
    "deqp_url": "",
    "deqp_rev": "ef7f512b6493029b011176c0bd5a2a7a1d7898d3"
}

deps = {
    "third_party/gyp": "http://gyp.googlecode.com/svn/trunk@1806",
    "tests/third_party/googletest": "http://googletest.googlecode.com/svn/trunk@629",
    "tests/third_party/googlemock": "http://googlemock.googlecode.com/svn/trunk@410",
}

gyp_cmd = [ "python", "build/gyp_angle" ]

if len(Var("deqp_url")) > 0:
    deps["tests/third_party/deqp"] = Var("deqp_url") + '@' + Var("deqp_rev")
    gyp_cmd.append("-D angle_build_deqp_tests=true")

hooks = [
    {
        # A change to a .gyp, .gypi, or to GYP itself should run the generator.
        "pattern": ".",
        "action": gyp_cmd,
    },
]
