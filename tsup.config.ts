import { defineConfig } from "tsup";

export default defineConfig(() => ({
  dts: true,
  format: [
    "esm",
    "cjs",
  ],
  entry: {
    main: "lib/main.ts",
  },
  external: [
    "electron",
  ],
}));
