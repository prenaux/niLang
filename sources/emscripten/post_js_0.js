// Here you can extend some APIs to use from C++
// The format is:
// niModuleName = { // here all the extra functions }
niExtensions = {
  niCURL: {
    // This functions allow the JavaScript code to intercept a fetch request
    // coming from C++ niCurl library and overrides it for anything you want.
    // The return value will be sent back to the client. It should be
    // a JSON with the fields "url", "headers" and "payload".
    // Example:
    //   handleFetchOverride: function () {
    //     console.log("Module.niCURL: handleFetchOverride");
    //     return `
    //         {
    //           "url": "http://example.com",
    //           "headers": {
    //             "Content-Type": "application/json",
    //             "Accept": "application/json"
    //           },
    //           "payload": {
    //             "status": "OK",
    //             "data": "this will be send back to client in cml format"
    //           }
    //         }`
    //   }
    handleFetchOverride: null
  }
}

// merge extensions into the Module library
// XXX: We should use mergeInto instead but for some reason cannot find it
//mergeInto(Module, niExtensions);
Object.assign(Module, niExtensions);
