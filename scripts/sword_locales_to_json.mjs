import fs from 'fs';
import https from 'https';
import { iso6393 } from 'iso-639-3';

const EZRA_LOCALES_URL = 'https://raw.githubusercontent.com/ezra-bible-app/ezra-bible-app/parse-sword-languages/locales/locales.json';
// const EZRA_LOCALES_URL = 'https://raw.githubusercontent.com/ezra-bible-app/ezra-bible-app/master/locales/locales.json';
const SWORD_LOCALES = "locales.d/locales.conf";

const extraLanguageCodes = ['cek', 'cth', 'dnj', 'esg', 'iqw', 'izz', 'ncq'];
var indexedLangs;

// https://nodejs.org/dist/latest-v14.x/docs/api/intl.html#intl_detecting_internationalization_support
const hasFullICU = (() => {
  try {
    const ukrainian = new Intl.DisplayNames('uk', { type: 'language', fallback: 'none' });
    return ukrainian.of('en') === 'англійська';
  } catch (err) {
    return false;
  }
})();
console.log(`Running on node version ${process.version}. Full ICU support: ${hasFullICU}`);

if (!isTesting()) {
  indexedLangs = indexIso6393Langs();

  const languages = parseSwordLocales();

  const allLocales = await getEzraBibleAppLocales(EZRA_LOCALES_URL);

  for (const code in languages) {
    languages[code] = addI18nData(code, allLocales, languages[code]);

    if (code.length < 4) {
      languages[code] = addIso639Details(code, languages[code]);
    }
  }

  const extraLanguages = getLanguageDetails(extraLanguageCodes, allLocales);

  saveToJsonFile({ ...languages, ...extraLanguages }, `lib/languages.json`);
}

/**
 * Converts SWORD locales.conf to object/JSON:
 * SWORD locale data:
 *   ...
 *   az-Cyrl=Азәрбајҹан
 *   az=Azərbaycan / Азәрбајҹан / آذربایجان
 *   az-Arab=آذربایجان
 *   az.en=Azerbaijani
 *   az-Latn=Azərbaycan
 *   ...
 * will be converted to
 *   "az": {
 *     "name": "Azərbaycan",
 *     "en": "Azerbaijani",
 *     "scripts": [ "Cyrl", "Arab", "Latn" ]
 *   }
 * 
 * @returns {Object} Map of languages by code
 */
function parseSwordLocales() {
  var languages = {};
  var isTextSection = false;

  try {
    const data = fs.readFileSync(SWORD_LOCALES, 'utf-8');
    const lines = data.split(/\r?\n/);

    for (const line of lines) {
      if (line.trim() == '[Text]') {
        isTextSection = true;
        continue;
      }
      if (!isTextSection || line[0] == '#' || line.trim() == '') {
        continue;
      }

      const data = parseLine(line.trim());
      if (!data) {
        continue;
      }

      languages[data.code] = addDataToMap(languages[data.code] || {}, data.name, data.script, data.locale, data.region);
    }
  } catch (err) {
    console.error(err);
  }
  return languages;
}

function parseLine(line) {
  const found = line.match(/^(?<code>[a-z0-9]{2,})(?:-(?<script>[a-z]{4}))?(?:-(?<region>[a-z]{2}))?(?:\.(?<locale>[a-z]{2,3}))?=(?<name>.+)$/i);
  if (!found) {
    console.log(`Skipping line: "${line}"`);
    return undefined;
  }

  const nameParts = found.groups.name.split('/'); // We want only the first part from the strings like "Azərbaycan / Азәрбајҹан / آذربایجان"
  const name = nameParts[0].trim() || found.groups.name; // fallback for the strings like "//Ani"

  return {
    code: found.groups.code,
    script: found.groups.script,
    region: found.groups.region,
    locale: found.groups.locale,
    name,
  };
}

function addDataToMap(langObj, name, script = undefined, locale = undefined, region = undefined) {
  const nameObj = locale ? { [locale]: name } : { 'name': name };
  if (script) {
    if (!langObj.scripts || langObj.scripts && !langObj.scripts.includes(script)) {
      langObj = {
        ...langObj,
        scripts: langObj.scripts ? [...langObj.scripts, script] : [script]
      };
    }
  } else if (region) { // region will be skipped if script is present. So far SWORD data has either script or region present
    langObj = {
      ...langObj,
      regions: {
        ...langObj.regions,
        [region]: nameObj
      }
    };
  } else {
    langObj = { ...langObj, ...nameObj };
  }

  return langObj;
}

function addIso639Details(code, data = {}) {
  if (indexedLangs[code]) {
    return {
      ...indexedLangs[code],
      ...data,
    };
  }
  console.log(`Couldn't find "${code}" in iso639-3 package`);
  return data;
}

function indexIso6393Langs() {
  var indexedLangs = {};

  for (const currentLang of iso6393) {
    if (currentLang.iso6391) {
      indexedLangs[currentLang.iso6391] = currentLang;
    }
    if (currentLang.iso6393) {
      indexedLangs[currentLang.iso6393] = currentLang;
    }
  }

  return indexedLangs;
}

function addI18nData(code, localeCodes, data = {}) {
  // Try to get localized name through standard Internationalization API, requires node >= 14
  // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/DisplayNames/of
  if (hasFullICU) {
    for (const localeCode of localeCodes) {

      if (code.length >= 2 && code.length <= 3) {
        const languagesInLocale = new Intl.DisplayNames(localeCode, { type: 'language', fallback: 'none' });
        const languageName = languagesInLocale.of(code);
        if (languageName) {
          data[localeCode] = toTitleCase(languageName);
        }
      } else if (code.length == 4) {
        const languageScript = (new Intl.DisplayNames(localeCode, { type: 'script', fallback: 'none' })).of(code);
        if (languageScript) {
          data[localeCode] = languageScript;
        }
      } else {
        console.log(`Non standard language code "${code}" while trying to i18n in "${localeCode}"`);
      }

      if (data.regions) {
        for (const regionCode in data.regions) {
          const languageRegion = (new Intl.DisplayNames(localeCode, { type: 'region', fallback: 'none' })).of(regionCode);
          if (languageRegion) {
            data.regions[regionCode][localeCode] = languageRegion;
          }
        }
      }

    }
  }
  return data;
}

async function getEzraBibleAppLocales(url) {
  return new Promise((resolve, reject) => {
    https.get(url, (res) => {
      const { statusCode } = res;

      let error;
      // Any 2xx status code signals a successful response but
      // here we're only checking for 200.
      if (statusCode !== 200) {
        error = new Error(`Request ${url} failed.\n` +
          `Status Code: ${statusCode}`);
      }
      if (error) {
        console.error(error.message);
        // Consume response data to free up memory
        res.resume();
        reject(error);
        return;
      }

      res.setEncoding('utf8');
      let rawData = '';
      res.on('data', (chunk) => { rawData += chunk; });
      res.on('end', () => {
        try {
          const parsedData = JSON.parse(rawData);
          console.log(parsedData.available);
          resolve(parsedData.available);
        } catch (err) {
          console.error(err.message);
          reject(err);
        }
      });
    }).on('error', (err) => {
      console.error(`Got error: ${err.message}`);
      reject(err);
    });
  });
}

function toTitleCase(str) {
  return str.slice(0, 1).toLocaleUpperCase() + str.slice(1);
}

function saveToJsonFile(data, filename) {
  const jsonData = JSON.stringify(data);
  // const jsonData = JSON.stringify(data, null, 2); // If pretty print is needed
  fs.writeFileSync(filename, jsonData);
}

function getLanguageDetails(languageCodes, allLocales) {
  var languages = {};
  for (const code of languageCodes) {
    languages[code] = addIso639Details(code);
    languages[code] = addI18nData(code, allLocales, languages[code]);
  }
  return languages;
}

function isTesting() {
  return process.env.NODE_ENV == 'test' || process.env.JEST_WORKER_ID !== undefined;
}
