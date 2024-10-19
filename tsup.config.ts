import { defineConfig } from "tsup";

export default defineConfig(() => ({
  dts: true,
  format: "esm",
  entry: {
    main: "lib/main.ts",
  },
  external: [
    "electron",
  ],
}));
