// Here you can extend some APIs to use from C++
// The format is:
// niModuleName: { // here all the extra functions }
niExtensions = {
  niCURL: {
    // this function is used to filter the url the JavaScript code wants to fetch.
    // Return true to execute your own fetch in handleFetchOverride
    // Return false if you want a normal emscripten fetch.
    shouldOverrideFetch: function(url) {
      return false;
    },
    // This function allows the JavaScript code to intercept a fetch request
    // coming from C++ niCurl library and overrides it for anything you want.
    // The function receives these parameters:
    //  - aRequestUrl: The Url the client wants to fetch
    //  - onSuccess(json): A callback that returns a successful fetch result in json.
    //      The client will receive back a status 200 with the json as data.
    //  - onError(json): A callback that returns an error in json format.
    //      The client will receive back a status 500 with the json as data.
    //  - onProgress(json): A callback that return a Progress event to the client
    //      WIP
    // The JSON must have the following structure (example):
    //    {
    //      "url": "http://example.com",
    //      "headers": {
    //        "Content-Type": "application/json",
    //        "Accept": "application/json"
    //       },
    //       "payload": [{"id":"90","symbol":"BTC","name":"Bitcoin","nameid":"bitcoin","rank":1,"price_usd":"23864.25","percent_change_24h":"-2.25","percent_change_1h":"0.17","percent_change_7d":"-4.28","market_cap_usd":"460296162080.63","volume24":"28850217961.42","volume24_native":"1208930.23","csupply":"19288102.00","price_btc":"1.00","tsupply":"19288102","msupply":"21000000"}]
    //    }
    //
    // Example:
    //
    //    handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
    //      console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
    //      await new Promise(r => setTimeout(r, 10000));
    //      var result = `
    //           {
    //            "url": "http://example.com",
    //            "headers": {
    //              "Content-Type": "application/json",
    //              "Accept": "application/json"
    //            },
    //            "payload": [{"id":"90","symbol":"BTC","name":"Bitcoin","nameid":"bitcoin","rank":1,"price_usd":"23864.25","percent_change_24h":"-2.25","percent_change_1h":"0.17","percent_change_7d":"-4.28","market_cap_usd":"460296162080.63","volume24":"28850217961.42","volume24_native":"1208930.23","csupply":"19288102.00","price_btc":"1.00","tsupply":"19288102","msupply":"21000000"}]
    //          }`
    //      onSuccess(result);
    //    }
    handleFetchOverride: null
  }
}

// merge extensions into the Module library
Object.assign(Module, niExtensions);
