deps = {
    "angle/third_party/gyp": "http://gyp.googlecode.com/svn/trunk@1806",
    "angle/tests/third_party/googletest": "http://googletest.googlecode.com/svn/trunk@629",
    "angle/tests/third_party/googlemock": "http://googlemock.googlecode.com/svn/trunk@410",
}

hooks = [
    {
        # A change to a .gyp, .gypi, or to GYP itself should run the generator.
        "pattern": ".",
        "action": [ "python", "angle/build/gyp_angle" ],
    },
]
