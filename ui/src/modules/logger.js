export function logDebug(...args) {
  // console.log("Debug: env=", import.meta.env);
  if (import.meta.env.VITE_APP_DEBUG === undefined) return
  console.log('Debug', ...args)
}

export function logInfo(...args) {
  // if(process.env.VUE_APP_INFO === undefined)
  //  return

  console.log('Info', ...args)
}

export function logError(...args) {
  console.log('Error', ...args)
}
