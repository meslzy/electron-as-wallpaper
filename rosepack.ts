import {defineRosepack} from "rosepack";

export default defineRosepack((config) => ({
  clean: config.mode === "production",
  format: [
    "dts",
    "cjs",
  ],
  input: {
    main: "source/main.ts",
  },
}));