module.exports = {
  root: true,
  extends: [
    "@meslzy/eslint-config",
    "@meslzy/eslint-config/typescript",
  ],
  parserOptions: {
    project: true,
    tsconfigRootDir: __dirname,
  },
};