// Here you can extend some APIs to use from C++
// The format is:
// niModuleName: { // here all the extra functions }
niExtensions = {
  niCURL: {
    // This function allows the JavaScript code to intercept a fetch request
    // coming from C++ niCurl library and overrides it for anything you want.
    // The return value will be sent back to the niCURL client. It should be
    // a JSON with the fields "url", "headers" and "payload". url and headers are
    // self explanatory. Payload contains two fields:
    //  - status (string): A string that tells niCURL if the override was properly
    //    handled. It can be 3 different values:
    //    - OK: The JS code didn't encountered any problem and will put the result
    //          of the fetch as-is in the "payload" field.
    //    - ERROR: The JS code found a problem while trying to fetch the data.
    //    - SKIP: The JS code decided that is not gonna override the fetch. Normally
    //            this is because is not interested in the URL being fetched.
    //  - payload (string): Depends on the content of status:
    //    - In the case of "OK" status it contains the result of the fetch.
    //    - In the case of "ERROR" status it contains a description of the error.
    //    - In the case of "SKIP" status it contains an empty string.
    // Example:
    //   handleFetchOverride: function (aRequestUrl) {
    //     console.log("Module.niCURL: handleFetchOverride");
    //     return `
    //         {
    //           "status": "OK",
    //           "url": "http://example.com",
    //           "headers": {
    //             "Content-Type": "application/json",
    //             "Accept": "application/json"
    //           },
    //           "payload": "your fetch result here"
    //         }`
    //   }
    handleFetchOverride: null
  }
}

// merge extensions into the Module library
Object.assign(Module, niExtensions);
