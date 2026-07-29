import { sections } from "./presentation-data";
import { ReadingNavigation } from "./reading-navigation";
import { SectionVisual } from "./section-visual";

const sourceDocument =
  "https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md";

export default function Home() {
  const navigationItems = sections.map(({ id, index, label }) => ({ id, index, label }));

  return (
    <>
      <a className="skip-link" href="#content">
        Saltar al contenido
      </a>
      <ReadingNavigation items={navigationItems} />
      <main id="content">
        {sections.map((section) => (
          <section
            aria-labelledby={`${section.id}-title`}
            data-presentation-section
            id={section.id}
            key={section.id}
          >
            <p>Capítulo {String(section.index).padStart(2, "0")}</p>
            <h2 id={`${section.id}-title`}>{section.title}</h2>
            <p>{section.thesis}</p>
            <SectionVisual kind={section.visual} />
            <ul>
              {section.bullets.map((bullet) => (
                <li key={bullet}>{bullet}</li>
              ))}
            </ul>
            {section.detail ? (
              <details>
                <summary>Profundizar</summary>
                <p>{section.detail}</p>
              </details>
            ) : null}
          </section>
        ))}
        <footer>
          <a href={sourceDocument}>Leer el documento técnico completo</a>
        </footer>
      </main>
    </>
  );
}
